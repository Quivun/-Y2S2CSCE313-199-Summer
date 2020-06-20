#include "common.h"
#include "BoundedBuffer.h"
#include "Histogram.h"
#include "common.h"
#include "HistogramCollection.h"
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

FIFORequestChannel *create_new_channel(FIFORequestChannel *mainchan)
{
    int bufferSize = 1024;
    char name[bufferSize];
    MESSAGE_TYPE m = NEWCHANNEL_MSG;
    mainchan->cwrite(&m, sizeof(m));
    mainchan->cread(name, bufferSize);
    FIFORequestChannel *newchan = new FIFORequestChannel(name, FIFORequestChannel::CLIENT_SIDE);
    return newchan;
}
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

void workers_thread_function(FIFORequestChannel *chan, BoundedBuffer *request_buffer, HistogramCollection *hc)
{
    int bufferSize = 1024;
    char buf[bufferSize];
    double resp = 0;
    while (true)
    {
        request_buffer->pop(buf, bufferSize);
        MESSAGE_TYPE *m = (MESSAGE_TYPE *)buf;
        if (*m = DATA_MSG)
        {
            cout << "Data" << endl;
            // Send to data channel
            chan->cwrite(buf, sizeof(datamsg));
            chan->cread(&resp, sizeof(double));
            hc->update(((datamsg *)buf)->person, resp);
        }
        else if (*m == FILE_MSG)
        {
            cout << "File" << endl;
            // TBD File Message
        }
        else if (*m == QUIT_MSG)
        {
            cout << "Quit" << endl;
            // Send quit
            // chan->cwrite((char *)&m, sizeof(MESSAGE_TYPE));
            chan->cwrite(m, sizeof(MESSAGE_TYPE));
            delete chan;
            // Because of this we don't need to cleanup within the main.
            break;
        }
    }
    /*
		Functionality of the workers threads	
    */
}

int main(int argc, char *argv[])
{
    int opt;
    int n = 15000;       //default number of requests per "patient"
    int p = 1;           // number of patients [1,15]
    int w = 200;         //default number of workers threads
    int b = 500;         // default capacity of the request buffer, you should change this default
    int m = MAX_MESSAGE; // default capacity of the message buffer
    while ((opt = getopt(argc, argv, "n:p:w:b:m:")) != -1)
    {
        switch (opt)
        {
        case 'n':
            n = atoi(optarg);
            break;
        case 'p':
            p = atoi(optarg);
            break;
        case 'w':
            w = atoi(optarg);
            break;
        case 'b':
            b = atoi(optarg);
            break;
        case 'm':
            m = atoi(optarg);
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

    srand(time_t(NULL));

    int pid = fork();
    if (pid == 0)
    {
        // modify this to pass along m
        execl("server", "server", (char *)NULL);
    }

    FIFORequestChannel *chan = new FIFORequestChannel("control", FIFORequestChannel::CLIENT_SIDE);
    BoundedBuffer request_buffer(b);
    HistogramCollection hc;

    for (int q = 0; q < p; q++)
    {
        Histogram *h = new Histogram(10, -2.0, 2.0);
        hc.add(h);
    }

    // Make workers channels
    cout << "Making Workers channels" << endl;
    FIFORequestChannel *wchans[w];
    for (int q = 0; q < w; q++)
    {
        wchans[q] = create_new_channel(chan);
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
    cout << "Workers start..." << endl;
    thread workers[w];
    for (int q = 0; q < w; q++)
    {
        workers[q] = thread(workers_thread_function, wchans[q], &request_buffer, &hc);
    }
    cout << "Workers complete!" << endl;

    /* Join all threads here */
    cout << endl
         << "Joining threads" << endl;
    cout << "Patient start..." << endl;

    for (int q = 0; q < p; q++)
    {
        cout << q << endl;
        patient[q].join();
    }
    cout << "Patient complete!" << endl;
    // They will now see the quit message.
    cout << "Sending Quit Test Start" << endl;
    for (int q = 0; q < w; q++)
    {
        MESSAGE_TYPE quit = QUIT_MSG;
        request_buffer.push((char *)&quit, sizeof(quit));
    }
    cout << "Sending Quit Test End" << endl;
    //
    cout << "Workers start..." << endl;

    for (int q = 0; q < w; q++)
    {
        cout << q << endl;
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
    /*
    int secs = (end.tv_sec * 1e6 + end.tv_usec - start.tv_sec * 1e6 - start.tv_usec) / (int)1e6;
    int usecs = (int)(end.tv_sec * 1e6 + end.tv_usec - start.tv_sec * 1e6 - start.tv_usec) % ((int)1e6);
    cout << "Took " << secs << " seconds and " << usecs << " micro seconds" << endl;
    */

    // Cleaning up the workers channels
    /*
    for (int q = 0; q < w; q++)
    {
        MESSAGE_TYPE quit = QUIT_MSG;
        wchans[q]->cwrite((char *)&quit, sizeof(MESSAGE_TYPE));
        delete wchans[q];
    }
    // Done so within the workers thread function via quit message.
    */
    // Cleaning up the main channel
    cout << "Deleting main channel" << endl;
    MESSAGE_TYPE quit = QUIT_MSG;
    chan->cwrite((char *)&quit, sizeof(MESSAGE_TYPE));
    cout << "Client side complete!" << endl;
    delete chan;
}
