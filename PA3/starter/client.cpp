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
void patient_thread_function(/*add necessary arguments*/ int dataNum, int patientNum, FIFORequestChannel *chan, HistogramCollection *hc)
{
    /* What will the patient threads do? */
    int timestamp = 0;
    int ecg = 1;
    double resp = 0;
    datamsg d(patientNum, timestamp, ecg);

    for (int q = 0; q < dataNum; q++)
    {
        chan->cwrite(&d, sizeof(datamsg));
        chan->cread(&resp, sizeof(double));
        hc->update(patientNum, resp);
        d.seconds += 0.004;
    }
}

void worker_thread_function(/*add necessary arguments*/)
{
    /*
		Functionality of the worker threads	
    */
}

int main(int argc, char *argv[])
{
    int opt;
    int n = 1000;        //default number of requests per "patient"
    int p = 10;          // number of patients [1,15]
    int w = 100;         //default number of worker threads
    int b = 20;          // default capacity of the request buffer, you should change this default
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
    cout << "Number of Worker Threads : " << w << endl;
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

    // Make worker channels
    FIFORequestChannel *wchans[p];
    for (int q = 0; q < p; q++)
    {
        wchans[q] = create_new_channel(chan);
    }

    struct timeval start, end;
    gettimeofday(&start, 0);

    /* Start all threads here */
    thread patient[p];
    for (int q = 0; q < p; q++)
    {
        patient[q] = thread(patient_thread_function, n, q + 1, wchans[q], &hc);
    }

    /* Join all threads here */
    for (int q = 0; q < p; q++)
    {
        patient[q].join();
    }
    gettimeofday(&end, 0);
    // print the results
    hc.print();
    int secs = (end.tv_sec * 1e6 + end.tv_usec - start.tv_sec * 1e6 - start.tv_usec) / (int)1e6;
    int usecs = (int)(end.tv_sec * 1e6 + end.tv_usec - start.tv_sec * 1e6 - start.tv_usec) % ((int)1e6);
    cout << "Took " << secs << " seconds and " << usecs << " micro seconds" << endl;

    // Cleaning up the worker channels
    for (int q = 0; q < p; q++)
    {
        MESSAGE_TYPE q = QUIT_MSG;
        wchans[q]->cwrite((char *)&q, sizeof(MESSAGE_TYPE));
        delete wchans[q];
    }
    // Cleaning up the main channel
    MESSAGE_TYPE q = QUIT_MSG;
    chan->cwrite((char *)&q, sizeof(MESSAGE_TYPE));
    cout << "All Done!!!" << endl;
    delete chan;
}
