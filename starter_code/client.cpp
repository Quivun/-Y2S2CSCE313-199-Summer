

/*
    Tanzir Ahmed
    Department of Computer Science & Engineering
    Texas A&M University
    Date  : 2/8/20
 */
#include "common.h"
#include "FIFOreqchannel.h"
#include <cstring>
using namespace std;


int main(int argc, char *argv[]){
    int n = 100;    // default number of requests per "patient"
	int p = 15;		// number of patients
    srand(time_t(NULL));
    int option;
    int patient;
    int ecg;
    bool pflag = 0;
    bool tflag = 0;
    bool eflag = 0;
    bool fflag = 0;
    bool cflag = 0;
    bool mflag = 0;
    int buffer_size = 256;
    bool failflag = 0;
    double time;
    string filename = "";
    while ((option = getopt(argc, argv,"p:t:e:f:m:c"))!= -1){
        switch(option)
        {
            case 'p':
                pflag = 1;
                patient = atoi(optarg);
                cout << "Patient: " << patient << endl;
                break;
            case 't':
                tflag = 1;
                time = atof(optarg);
                cout << "Time: " << time << endl;
                break;
            case 'e':
                eflag = 1;
                ecg = atoi(optarg);
                cout <<"ECG #: " << ecg << endl;
                break;
            case 'f':
                fflag = 1;
                filename = optarg;
                cout << "Filename: " << filename << endl;
                break;
            case 'c':
                cflag = 1;
                break;
            case 'm':
                mflag = 1;
                buffer_size = atoi(optarg);
                cout << "Buff Cap: " << buffer_size << endl;
                break;
            case '?':
                failflag = 1;
                cout << "Unknown input" << endl;
                break;
        }
    }
    char* args[] = {"./server", nullptr};
    if (fork()==0){
        execvp(args[0],argv);
    }
    FIFORequestChannel chan ("control", FIFORequestChannel::CLIENT_SIDE);
    if (pflag&&eflag&&tflag){
        datamsg data = datamsg(patient, time, ecg);
        chan.cwrite(&data,sizeof(data));
        double data1;
        chan.cread((char*)&data1,sizeof(double));
        cout << "Response: " << data1 << endl;
    }
    else if (pflag && !tflag && !eflag){
    ofstream myfile;
    myfile.open("received/x1.csv");
    double i = 0;
    struct timeval start, end;
    gettimeofday(&start, nullptr);
    while (i < 59.996){
        myfile << i << ",";
        datamsg d1 = datamsg(patient, i, 1);
        datamsg d2 = datamsg(patient, i, 2);
        chan.cwrite(&d1,sizeof(d1));
        double data1;
        chan.cread((char*)&data1,sizeof(double));
        myfile << data1 << ",";
        chan.cwrite(&d2,sizeof(d2));
        double data2;
        chan.cread((char*)&data2,sizeof(double));
        myfile << data2 << endl;
        i = i + 0.004;
    }
    myfile.close();
    gettimeofday(&end, nullptr);
    double time_spent = (end.tv_sec - start.tv_sec)*1e6;
    time_spent = (time_spent + (end.tv_usec -start.tv_usec))*1e-6;
    cout << "Time taken to retrieve data points: " << time_spent << "sec" << endl;
    }
    else if (fflag && !mflag){
        struct timeval start, end;
        cout << "Buffer size: " << MAX_MESSAGE << endl;
        char* buffer = new char[sizeof(filemsg)+filename.size()];
        filemsg file = filemsg(0, 0);
        memcpy(buffer, (char*)&file, sizeof(filemsg));
        strcpy(buffer+sizeof(filemsg),filename.c_str());
        chan.cwrite(buffer, sizeof(filemsg)+filename.size()+1);
        int64_t file_length;
        int offset = 0;
        chan.cread((char*)&file_length,sizeof(int64_t));
        cout << "File length: " << file_length << endl;
        int max_requests = ceil(file_length/MAX_MESSAGE);
        cout << "Maximum requests: " << max_requests << endl; 
        gettimeofday(&start, nullptr);
        ofstream myfile;
        string out_file = "received/"+filename;
        myfile.open(out_file, ios::binary);
        int i = 0;
        while(i < max_requests){
            filemsg requestmsg = filemsg(offset, MAX_MESSAGE);
            char* buffer2 = new char[sizeof(requestmsg) + filename.size()];
            memcpy(buffer2, (char*)&requestmsg, sizeof(requestmsg));
            strcpy (buffer2+sizeof(requestmsg), filename.c_str());
            chan.cwrite(buffer2, sizeof(filemsg)+filename.size()+1);
            chan.cread(buffer2, MAX_MESSAGE);
            i++;
            offset += MAX_MESSAGE;
            myfile.write(buffer2, MAX_MESSAGE);
        }
        if (file_length - offset > 0){
            filemsg requestmsg = filemsg(offset, file_length - offset);
            char* buffer2 = new char[sizeof(requestmsg) + filename.size()+1];
            memcpy(buffer2, (char*)&requestmsg, sizeof(requestmsg));
            strcpy (buffer2+sizeof(requestmsg), filename.c_str());
            chan.cwrite(buffer2, sizeof(requestmsg)+filename.size()+1);
            chan.cread(buffer2, MAX_MESSAGE);
            myfile.write(buffer2, file_length-offset);
        }
        myfile.close();
        gettimeofday(&end, nullptr);
        double time_spent = (end.tv_sec - start.tv_sec)*1e6;
        time_spent = (time_spent + (end.tv_usec -start.tv_usec))*1e-6;
        cout << "Time taken to transfer a file: " << time_spent << "sec" << endl;
    }
    else if (mflag){
        cout << "Buffer size changed to: " << MAX_MESSAGE << endl;
        MESSAGE_TYPE m = QUIT_MSG;
        chan.cwrite (&m, sizeof (MESSAGE_TYPE));
        wait(NULL);
    }
    else if (fflag && mflag){
        struct timeval start, end;
        cout << "Buffer size: " << MAX_MESSAGE << endl;
        char* buffer = new char[sizeof(filemsg)+filename.size()];
        filemsg file = filemsg(0, 0);
        memcpy(buffer, (char*)&file, sizeof(filemsg));
        strcpy(buffer+sizeof(filemsg),filename.c_str());
        chan.cwrite(buffer, sizeof(filemsg)+filename.size()+1);
        int64_t file_length;
        int offset = 0;
        chan.cread((char*)&file_length,sizeof(int64_t));
        cout << "File length: " << file_length << endl;
        int max_requests = ceil(file_length/MAX_MESSAGE);
        cout << "Maximum requests: " << max_requests << endl;
        gettimeofday(&start, nullptr);
        ofstream myfile;
        string out_file = "received/"+filename;
        myfile.open(out_file, ios::binary);
        int i = 0;
        while(i < max_requests){
            filemsg requestmsg = filemsg(offset, MAX_MESSAGE);
            char* buffer2 = new char[sizeof(requestmsg) + filename.size()];
            memcpy(buffer2, (char*)&requestmsg, sizeof(requestmsg));
            strcpy (buffer2+sizeof(requestmsg), filename.c_str());
            chan.cwrite(buffer2, sizeof(filemsg)+filename.size()+1);
            chan.cread(buffer2, MAX_MESSAGE);
            i++;
            offset += MAX_MESSAGE;
            myfile.write(buffer2, MAX_MESSAGE);
        }
        if (file_length - offset > 0){
            filemsg requestmsg = filemsg(offset, file_length - offset);
            char* buffer2 = new char[sizeof(requestmsg) + filename.size()+1];
            memcpy(buffer2, (char*)&requestmsg, sizeof(requestmsg));
            strcpy (buffer2+sizeof(requestmsg), filename.c_str());
            chan.cwrite(buffer2, sizeof(requestmsg)+filename.size()+1);
            chan.cread(buffer2, MAX_MESSAGE);
            myfile.write(buffer2, file_length-offset);
        }
        myfile.close();
        gettimeofday(&end, nullptr);
        double time_spent = (end.tv_sec - start.tv_sec)*1e6;
        time_spent = (time_spent + (end.tv_usec -start.tv_usec))*1e-6;
        cout << "Time taken to transfer a file: " << time_spent << "sec" << endl;
    }
    if (cflag){
        cout << "Opening a new channel" << endl;
        newchannelmsg newchannel = newchannelmsg();
        chan.cwrite((char*)&newchannel, sizeof(newchannelmsg));
        FIFORequestChannel newchan ("data1_", FIFORequestChannel::CLIENT_SIDE);
        datamsg data = datamsg(1, 30, 2);
        newchan.cwrite(&data,sizeof(data));
        double data1;
        newchan.cread((char*)&data1,sizeof(double));
        cout << "Response: " << data1 << endl;
        MESSAGE_TYPE m = QUIT_MSG;
        newchan.cwrite(&m, sizeof(MESSAGE_TYPE));
        chan.cwrite(&m, sizeof(MESSAGE_TYPE));
        wait(NULL);
    }
    else if (failflag){
        MESSAGE_TYPE m = QUIT_MSG;
        chan.cwrite (&m, sizeof (MESSAGE_TYPE));
        wait(NULL);
    }
}

