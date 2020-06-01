/*
    Joshua Chong
    CS Undergraduate
    Texas A&M University
    Date  : 5/31/20
 */
#include "common.h"
#include "FIFOreqchannel.h"

#include<iostream>
#include<string>
#include<stdio.h>
using namespace std;


int main(int argc, char *argv[]){
      int opt;
      int person = 0;
      double time = 0;
      int ecgType = 0;
      string fileName = "x1.csv";
      bool newChannel = false;
      string bufCap = MAX_MESSAGE;
    while ((opt = getopt(argc, argv, "p:t:e:f:c:m")) != -1)
    {
        switch (opt)
        {
        case 'p':
            person = atoi(optarg);
            break;
        case 't':
            time = stod(optarg);
            break;
        case 'e':
            ecgType = atoi(optarg);
            break;
        case 'f':
            fileName = optarg;
            break;
        case 'c':
            newChannel = true;
            break;
        case 'm':
            bufCap = optarg;
        case '?':
            cout << "Incorrect input" << endl;
            break;
        default:
            cout << "No input" << endl;
        }
    }
    cout <<"This is a person" <<  person << endl;
    int pid = fork();
    if (pid == 0){
        // Got points off last time I didn't auto summon rip
        char *args[] = { string("./server -m " + bufCap).c_str(), NULL};
        execvp(args[0], args);
    } else {
        int numPat = 10;
        int numReq = 100;
    }
    
    FIFORequestChannel chan ("control", FIFORequestChannel::CLIENT_SIDE);

    srand(time_t(NULL));
    // Start 1 Data point test 
    // sending a non-sense message 
    struct timeval s0,e0;
    gettimeofday(&s0, NULL);
    datamsg *nonSMsg = new datamsg(1,0.004,2);
    chan.cwrite( nonSMsg, sizeof(datamsg) );
    double data = 0;
    chan.cread(&data, sizeof(double));
    printf("Data = %lf", data);
    delete nonSMsg;
    gettimeofday(&e0,NULL);
    // Stop 1 Data point test

    // Start Data Retrieval test
    struct timeval s1,e1;
    gettimeofday(&s1, NULL);
    // offset = 0, length = 0
    filemsg *f0 = new filemsg(0,0);
    string fNameReq = "1.csv";
    string fNameOut = "x1.csv";
    int req = sizeof(filemsg) + fNameReq.size() + 1;
    char *buf = new char[req];
    memcpy(buf, f0, sizeof(filemsg));
    strcpy(buf+sizeof(filemsg), fNameReq.c_str());
    chan.cwrite(buf, req);

    __int64_t fSize;
    chan.cread(&fSize, sizeof(__int64_t));
    
    string output_path = string("received/" + fNameOut);
    FILE *f = fopen(output_path.c_str(), "wb");
    char *receiver = new char[MAX_MESSAGE];
    int reqAmt = 1000;
    while (fSize > 0 && reqAmt > 0){
        int req_len = min((__int64_t)MAX_MESSAGE, fSize);
        ((filemsg *)buf)->length = req_len;
        chan.cwrite(buf, req);
        chan.cread(receiver, req_len);
        fwrite(receiver, 1, req_len, f);
        ((filemsg *)buf)->offset += req_len;
        fSize -= req_len;
        reqAmt--;
    }
    fclose(f);
    delete buf;
    delete receiver;
    gettimeofday(&e1, NULL);
    // Stop Data Retrieval test

    MESSAGE_TYPE m = QUIT_MSG;
    chan.cwrite(&m, sizeof(MESSAGE_TYPE));

}
