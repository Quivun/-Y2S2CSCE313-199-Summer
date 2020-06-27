
#include "common.h"
#include "BoundedBuffer.h"
#include "Histogram.h"
#include "common.h"
#include "HistogramCollection.h"
#include "TCPreqchannel.h"
#include "FIFOreqchannel.h"
#include <time.h>
#include <thread>
using namespace std;

void timediff(struct timeval &start, struct timeval &end)
{
	int secs = (end.tv_sec * 1e6 + end.tv_usec - start.tv_sec * 1e6 - start.tv_usec) / (int)1e6;
	int usecs = (int)(end.tv_sec * 1e6 + end.tv_usec - start.tv_sec * 1e6 - start.tv_usec) % ((int)1e6);
	cout << "Took " << secs << " seconds and " << usecs << " micro seconds" << endl;
}
/*
TCPRequestChannel *create_new_channel(TCPRequestChannel *mainchan)
{
	int bufferSize = 1024;
	char name[bufferSize];
	MESSAGE_TYPE m = NEWCHANNEL_MSG;
	mainchan->cwrite(&m, sizeof(m));
	mainchan->cread(name, bufferSize);
	TCPRequestChannel *newchan = new TCPRequestChannel(name, FIFORequestChannel::CLIENT_SIDE);
	return newchan;
}
*/
void patient_thread_function(int dataNum, int patientNum, BoundedBuffer *request_buffer)
{
	/* What will the patient threads do? */
	double timestamp = 0;
	int ecg = 1;
	double resp = 0;
	datamsg d(patientNum, timestamp, ecg);

	for (int q = 0; q < dataNum; q++)
	{
		/*chan->cwrite(&d, sizeof(datamsg));
        chan->cread(&resp, sizeof(double));
        hc->update(patientNum, resp);
        */
		// Done by workers threads as they only have access to fifo requeest channel
		request_buffer->push((char *)&d, sizeof(datamsg));
		d.seconds += 0.004;
	}
}

void workers_thread_function(TCPRequestChannel *chan, BoundedBuffer *request_buffer, HistogramCollection *hc, int bufCap)
{
	int bufferSize = 1024;
	char buf[bufferSize];
	double resp = 0;
	char recievedBuf[bufCap];
	while (true)
	{
		request_buffer->pop(buf, bufferSize);
		MESSAGE_TYPE *m = (MESSAGE_TYPE *)buf;
		if (*m == DATA_MSG)
		{
			// Send to data channel
			chan->cwrite(buf, sizeof(datamsg));
			chan->cread(&resp, sizeof(double));
			hc->update(((datamsg *)buf)->person, resp);
		}
		else if (*m == QUIT_MSG)
		{
			// Send quit
			// chan->cwrite((char *)&m, sizeof(MESSAGE_TYPE));
			chan->cwrite(m, sizeof(MESSAGE_TYPE));
			delete chan;
			// Because of this we don't need to cleanup within the main.
			break;
		}
		else if (*m == FILE_MSG)
		{
			filemsg *fm = (filemsg *)buf;
			string fname = (char *)(fm + 1);
			int sz = sizeof(filemsg) + fname.size() + 1; // Rather than just sizeof, it requires the size also
			chan->cwrite(buf, sz);
			chan->cread(recievedBuf, bufCap);
			// TBD File Message
			string recievedFname = "recv/" + fname;
			FILE *fp = fopen(recievedFname.c_str(), "r+");
			// r+ mode so the system knows to open the file as both read and write mode. Won't destroy existing content of the file so we can navigate to a specified index. PreReq is that the file is expected to be there. We can sometimes get the files out of order. Self Question : Demux it to order it?
			fseek(fp, fm->offset, SEEK_SET);
			fwrite(recievedBuf, 1, fm->length, fp);
			fclose(fp);
		}
	}
	/*
		Functionality of the workers threads	
    */
}

/* // naiveSol
void file_thread_function(TCPRequestChannel* chan, string fname, int m, BoundedBuffer* request_buffer){
    filemsg f(0,0);
    char buf[1000];
    memcpy (buf, &f, sizeof (f));
    strcpy(buf + sizeof(filemsg) + fname.size() +1 );
    __int64_t len;
    chan->cread(&len, sizeof(__int64_t));
    //Create the file and preallocate it to the desired length
    string recfilename = "recv/" + fname;
    cout << "Going to create a file" << recfilename << " of length " << len << endl;
    int ret = truncate (recfilename.c_str(), len);
    if (ret < 0){
        perror ("Truncate Error : ");
        exit(0);
    }
    filemsg* fm = (filemsg* ) buf;
    __int64_t remlen = len;
    while (remlen > 0){
        fm->length = min (remlen, (__int64_t) m );
        //chan->cwrite (buf, sizeof (filemsg) + fname.size() + 1);
        // chan->cread (recvbuf, m);
        request_buffer->push(buf,sizeof(filemsg) + fname.size() + 1);
        fm -> offset += fm-> length;
        remlen -= fm->length;
    }
}
*/
void file_thread_function(string fname, BoundedBuffer *request_buffer, TCPRequestChannel *chan, int bufCap)
{
	// 1. Create the file
	string recievedFname = "recv/" + fname;
	// Pre allocate the length of the file to make it as long as the original length
	int bufferSize = 1024;
	char buf[bufferSize];
	filemsg f(0, 0);
	memcpy(buf, &f, sizeof(f));
	strcpy(buf + sizeof(f), fname.c_str());
	chan->cwrite(buf, sizeof(f) + fname.size() + 1); // Sends message to server
	__int64_t filelength;
	chan->cread(&filelength, sizeof(filelength));

	FILE *fp = fopen(recievedFname.c_str(), "w");
	fseek(fp, filelength, SEEK_SET);
	fclose(fp);
	// 2. Generate all the file messages
	filemsg *fm = (filemsg *)buf;
	__int64_t remlen = filelength;
	while (remlen > 0)
	{
		fm->length = min(remlen, (__int64_t)bufCap);
		request_buffer->push(buf, sizeof(filemsg) + fname.size() + 1);
		fm->offset += fm->length;
		remlen -= fm->length;
	}
}
int main(int argc, char *argv[])
{
	int opt;
	int n = 15000;		 //default number of requests per "patient"
	int p = 1;			 // number of patients [1,15]
	int w = 200;		 //default number of workers threads
	int b = 500;		 // default capacity of the request buffer, you should change this default
	int m = MAX_MESSAGE; // default capacity of the message buffer
	string mm = to_string(MAX_MESSAGE);
	string fname = "1.csv";
	bool pflag = false;
	bool fflag = false;
	string host;
	string port;
	srand(time_t(NULL));

	while ((opt = getopt(argc, argv, "n:p:w:b:m:f:h:r:")) != -1)
	{
		switch (opt)
		{
		case 'n':
			n = atoi(optarg);
			break;
		case 'p':
			p = atoi(optarg);
			pflag = true;
			break;
		case 'w':
			w = atoi(optarg);
			break;
		case 'b':
			b = atoi(optarg);
			break;
		case 'm':
			mm = optarg;
			m = stoi(mm);

			break;
		case 'f':
			fname = optarg;
			fflag = true;
			break;
		case 'h':
			host = optarg;
			break;
		case 'r':
			port = optarg;
			break;
		case '?':
			cout << "Incorrect input detected." << endl;
			break;
		default:
			cout << "No input found." << endl;
			break;
		}
	}
	cout << "Number of Patient Requests : " << n << endl;
	cout << "Number of Patients :  " << p << endl;
	cout << "Number of workers Threads : " << w << endl;
	cout << "Capacity of Request Buffer : " << b << endl;
	cout << "Capacity of Message Buffer : " << m << endl;
	cout << "File requested : " << fname << endl;
	/* if (pflag)
    {
        cout << "Patients confirmed, will run patient threads." << endl;
    }
    else
    {
        cout << "No patients specified, will not run patient threads." << endl;
    }
    if (fflag)
    {
        cout << "File specified. Will run file threads." << endl;
    }
    else
    {
        cout << "No file specified, will not run file threads." << endl;
    }
    */
	/*
    int pid = fork();
    if (pid == 0)
    {
        // modify this to pass along m
        char *serverEntry = new char[mm.size() + 1];
        copy(mm.begin(), mm.end(), serverEntry);
        serverEntry[mm.size()] = '\0';

        char *args[] = {"./server", "m", serverEntry, NULL};
		// execl ("server", "server", "-m", to_string(m).c_str(), (char*)NULL);
        execvp(args[0], args);
    }
*/
	TCPRequestChannel *chan = new TCPRequestChannel(host,port, FIFORequestChannel::CLIENT_SIDE);
	BoundedBuffer request_buffer(b);
	HistogramCollection hc;

	for (int q = 0; q < p; q++)
	{
		Histogram *h = new Histogram(10, -2.0, 2.0);
		hc.add(h);
	}

	// Make workers channels
	cout << "Making Workers channels" << endl;
	TCPRequestChannel *wchans[w];
	for (int q = 0; q < w; q++)
	{
		wchans[q] = new TCPRequestChannel (host, port, FIFORequestChannel::CLIENT_SIDE);
		// wchans[q] = create_new_channel(chan);
	}

	struct timeval start, end;
	gettimeofday(&start, 0);

	cout << endl
		 << "Beginning thread creation" << endl;

	/* Start all threads here */
	cout << "Patient start..." << endl;
	thread patient[p];
	for (int q = 0; q < p; q++)
	{
		patient[q] = thread(patient_thread_function, n, q + 1, &request_buffer);
	}
	// Remember the patient threads are pushing, the workers threads are popping.
	cout << "Patient complete!" << endl;

	cout << "FileThreads start... " << endl;
	thread filethread(file_thread_function, fname, &request_buffer, chan, m);
	cout << "FileThreads complete!" << endl;

	cout << "Workers start..." << endl;
	thread workers[w];
	for (int q = 0; q < w; q++)
	{
		workers[q] = thread(workers_thread_function, wchans[q], &request_buffer, &hc, m);
	}
	cout << "Workers complete!" << endl;

	/* Join all threads here */
	cout << endl
		 << "Joining threads" << endl;

	cout << "Patient start..." << endl;

	for (int q = 0; q < p; q++)
	{
		patient[q].join();
	}
	cout << "Patient complete!" << endl;

	filethread.join();
	cout << "Patient threads/file thread finished" << endl;

	// They will now see the quit message.
	cout << "Sending Quit messages : Start" << endl;
	for (int q = 0; q < w; q++)
	{
		MESSAGE_TYPE quit = QUIT_MSG;
		request_buffer.push((char *)&quit, sizeof(quit));
	}
	cout << "Sending Quit messages : End" << endl;
	//
	cout << "Workers start..." << endl;

	for (int q = 0; q < w; q++)
	{
		workers[q].join();
	}
	cout << "Workers complete!" << endl;

	gettimeofday(&end, 0);
	// print the results
	cout << endl
		 << endl;

	hc.print();

	timediff(start, end);
	cout << endl
		 << endl;

	// Cleaning up the main channel
	cout << "Deleting main channel" << endl;
	MESSAGE_TYPE quit = QUIT_MSG;
	chan->cwrite((char *)&quit, sizeof(MESSAGE_TYPE));
	cout << "Client side complete!" << endl;
	delete chan;
}

/*
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
using namespace std;
// q
// this is a made up communication protocol 
//	between the client and server. In this protocol
//	the client sends a number to the server, which the 
//	server doubles and then return. This part has nothing
//	to do with your PA6 or any real client-server.
void talk_to_server (int sockfd){
	char buf [1024];
	while (true){
	    cout << "Type a number for the server: "; 
        int num; 
        cin>> num;
		if (send (sockfd, &num, sizeof (int), 0) == -1){
            perror("client: Send failure");
            break;
        }
        if (recv (sockfd, buf, sizeof (buf), 0) < 0){
            perror ("client: Receive failure");    
            exit (0);
        }
        cout << "The server sent back " << *(int *)buf << endl; 
	}
}

// a client that mainly connects to a server 

int client (char * server_name, char* port)
{
	struct addrinfo hints, *res;
	int sockfd;

	// first, load up address structs with getaddrinfo():
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	int status;
	//getaddrinfo("www.example.com", "3490", &hints, &res);
	if ((status = getaddrinfo (server_name, port, &hints, &res)) != 0) {
        cerr << "getaddrinfo: " << gai_strerror(status) << endl;
        return -1;
    }

	// make a socket:
	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sockfd < 0){
		perror ("Cannot create scoket");	
		return -1;
	}

	// connect!
	if (connect(sockfd, res->ai_addr, res->ai_addrlen)<0){
		perror ("Cannot Connect");
		return -1;
	}
	talk_to_server(sockfd);
	return 0;
}


int main (int ac, char** av){
	if (ac < 3){
        cout << "Usage: ./client <server name/IP> <port no>" << endl;
        exit (-1);
    }
	client (av [1], av [2]);
}

*/