#include "common.h"
#include "BoundedBuffer.h"
#include "Histogram.h"
#include "common.h"
#include "HistogramCollection.h"
#include "FIFOreqchannel.h"
#include <time.h>
#include <threads>
using namespace std;

//  Naive Code
void timediff (struct timeval& start, struct timeval& end){

}

FIFORequestChannel* create_new_channel( FIFORequestChannel* mainchan ){
    char name [1024];
    MESSAGE_TYPE m = NEWCHANNEL_MSG;
    mainchan->cwrite (&m, sizeof(m));
    mainchan->cread (name, 1024);
    FIFORequestChannel* newchan = new FIFORequestChannel (name, FIFORequestChannelL::CLIENT_SIDE);
    return newchan;

}





//
void patient_thread_function(/*add necessary arguments*/)
{
    /* What will the patient threads do? */
}

void worker_thread_function(/*add necessary arguments*/)
{
    /*
		Functionality of the worker threads	
    */
}

int main(int argc, char *argv[])
{
    int n = 10000;       //default number of requests per "patient"
    int p = 10;          // number of patients [1,15]
    int w = 100;         //default number of worker threads
    int b = 20;          // default capacity of the request buffer, you should change this default
    int m = MAX_MESSAGE; // default capacity of the message buffer
    // Implement a getopt.

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

    for (int q  = 0;  q < p; q ++){
        Histogram* h = new Histogram (10, -2.0, 2.0);
        hc.add(h);
    }

    //Make worker channels
    FIFORequestChannel* wchans[p];
    for (int q = 0; q < p; q++){
        wchans[q] = create_new_channel(chan);
    }


    struct timeval start, end;
    gettimeofday(&start, 0);

    /* Start all threads here */
    thread patient [p];
    for (int q = 0; q < p; q++){
        patient[q] = thread(patient_thread_function);
    }

    /* Join all threads here */
    for (int q = 0; q < p; q++){
        patient[q].join();
    }
    
    gettimeofday(&end, 0);
    // print the results
    hc.print();
    int secs = (end.tv_sec * 1e6 + end.tv_usec - start.tv_sec * 1e6 - start.tv_usec) / (int)1e6;
    int usecs = (int)(end.tv_sec * 1e6 + end.tv_usec - start.tv_sec * 1e6 - start.tv_usec) % ((int)1e6);
    cout << "Took " << secs << " seconds and " << usecs << " micro seconds" << endl;

    MESSAGE_TYPE q = QUIT_MSG;
    chan->cwrite((char *)&q, sizeof(MESSAGE_TYPE));
    cout << "All Done!!!" << endl;
    delete chan;
}
