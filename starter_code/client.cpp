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
      string bufCap = to_string(MAX_MESSAGE);
    while ((opt = getopt(argc, argv, "p:t:e:f:c:m")) != -1)
    {
        switch (opt)
        {
        case 'p':
            person = atoi(optarg);
            cout << "Patient : " << person << endl;
            break;
        case 't':
            time = stod(optarg);
            cout << "Time : " << time << endl;

            break;
        case 'e':
            ecgType = atoi(optarg);
            cout << "ECG Type : " << ecgType << endl;

            break;
        case 'f':
            fileName = optarg;
            cout << "Name of File : " << fileName << endl;

            break;
        case 'c':
            newChannel = true;
            cout << "New Channel? : " << newChannel << endl;

            break;
        case 'm':
            bufCap = optarg;
            cout << "Memory Capacity : " << bufCap << endl;

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
        /*
        bufCap = "./server -m " + bufCap;
        char *serverEntry = new char[bufCap.size() +1];
        copy(bufCap.begin(),bufCap.end(),serverEntry);
        serverEntry[bufCap.size()] = '\0';
        
        char *args[] = { serverEntry, NULL};
        */
       char *args[] = {"./server", NULL};
        execvp(args[0], args);
    } else {
        int numPat = 10;
        int numReq = 100;
    }
    
    FIFORequestChannel chan ("control", FIFORequestChannel::CLIENT_SIDE);

    srand(time_t(NULL));
    // Start 1 Data point test 
    // Sending Non Sense message : 1 Data point retrieval
    struct timeval s0,e0;
    gettimeofday(&s0, NULL);
    datamsg *nonSMsg = new datamsg(person,time,ecgType);
    chan.cwrite( nonSMsg, sizeof(datamsg) );
    double data = 0;
    chan.cread(&data, sizeof(double));
    printf("Data = %lf", data);
    delete nonSMsg;
    gettimeofday(&e0,NULL);
    cout << "Single point complete" << endl;
    // Stop 1 Data point test
    
    // Start 1000 Data point test

    struct timeval s1,e1;
    gettimeofday(&s1, NULL);
    ofstream mF;
    mF.open("received/x1.csv");
    int t = 0;
    while (t < 59.996){
        datamsg ec1 = datamsg(person, t, 1);
        datamsg ec2 = datamsg(person, t, 2);
        mF << t;
        double d1 = 0;
        chan.cread((char*)&d1,sizeof(double));
        mF << d1 << ",";
        chan.cwrite(&ec2,sizeof(ec2));
        double d2 = 0;
        chan.cread((char*)&d2, sizeof(double));
        mF << d2 << endl;
        t += 0.004;
    }
    mF.close();
    gettimeofday(&e1,NULL);
    cout << "1000 Data point complete" << endl;

    // Stop 1000 Data point test
    // Start File Retrieval test
    struct timeval s2,e2;
    gettimeofday(&s2, NULL);
    // offset = 0, length = 0
    filemsg *f0 = new filemsg(0,0);
    string fNameReq = "1.csv";
    string fNameOut = "x2.csv";
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
    while (fSize > 0 ){
        int req_len = min((__int64_t)MAX_MESSAGE, fSize);
        ((filemsg *)buf)->length = req_len;
        chan.cwrite(buf, req);
        chan.cread(receiver, req_len);
        fwrite(receiver, 1, req_len, f);
        ((filemsg *)buf)->offset += req_len;
        fSize -= req_len;
    }
    fclose(f);
    delete buf;
    delete receiver;
    gettimeofday(&e2, NULL);
    cout << "File retrieval complete" << endl;
    // Stop File Retrieval test

    MESSAGE_TYPE m = QUIT_MSG;
    chan.cwrite(&m, sizeof(MESSAGE_TYPE));

}
