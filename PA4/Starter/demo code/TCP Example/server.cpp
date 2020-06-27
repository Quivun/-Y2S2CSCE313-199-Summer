#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <thread>
#include "TCPreqchannel.h"
#include "FIFOreqchannel.h"
using namespace std;

/* 
// Does not have to be here because the intent doesn't have to be sent to the server we want 
void process_newchannel_request(TCPRequestChannel *_channel)
{
    nchannels++;
    string new_channel_name = "data" + to_string(nchannels) + "_";
    char buf[30];
    strcpy(buf, new_channel_name.size() + 1);
    _channel->cwrite(buf, new_channel_name.size() + 1);

    TCPRequestChannel *data_channel = new TCPRequestChannel(new_channel_name, TCPRequestChannel::SERVER_SIZE);
    thread thread_for_client(handle_process_loop, data_channel);
    thread_for_client.detach();
}
// We just need to call connect one more time from the client

*/

void populate_file_data(int person)
{
    // cout << "Populating for person " << person << endl;
    string filename = "BIMDC/" + to_string(person) + ".csv";
    char line[100];
    ifstream ifs(filename.c_str());
    if (ifs.fail())
    {
        EXITONERROR("Data file: " + filename + " does not exist in the BIMDC/ directory");
    }
    int count = 0;
    while (!ifs.eof())
    {
        line[0] = 0;
        ifs.getline(line, 100);
        if (ifs.eof())
            break;
        double seconds = stod(split(string(line), ',')[0]);
        if (line[0])
            all_data[person - 1].push_back(string(line));
    }
}
double get_data_from_memory(int person, double seconds, int ecgno)
{
    int index = (int)round(seconds / 0.004);
    string line = all_data[person - 1][index];
    vector<string> parts = split(line, ',');
    double sec = stod(parts[0]);
    double ecg1 = stod(part[1]);
    double ecg = stod(parts[2]);
    if (ecgno == 1)
    {
        return ecg1;
    }
    else
    {
        return ecg2;
    }
}

void process_file_request(TCPRequestChannel *rc, char *request)
{
    filemsg f = *(filemsg *)request;
    string filename = request + sizeof(filemsg);
    filename = "BIMDC/" + filename; // adding path prefix to request file name
    // cout << "Server received request for file " << filename << endl;
    if (f.offset == 0 && f.length == 0)
    {
        __int64_t fs = get_file_size(filename);
        rc->cwrite((char *)&fs, sizeof(__int64_t));
        return;
    }

    char *response = request;
    // assures that the client isn't requesting too large of a chunk
    if (f.length > bufCap)
    {
        cerr << "Client is requesting a chunk bigger than server's capacity" << endl;
        cerr << "Returning nothing (i.e."
    }
    FILE *fp = fopen(filename.c_str(), "rb");
    if (!fp)
    {
        cerr << "Server recieved request for file: " << filename << " which cannot be opened." << endl;
        rc->cwrite(buffer, 0);
        return;
    }
    fseek(fp, f.offset, SEEK_SET);
    int nbytes = fread(response, 1, f.length, fp);
    // assues that the client is asking for the right number of bytes and this is especially importatnt for the last chucnk of the file when the remaining length is lesser than the buffer capacity of the client
    assert(nbytes == f.length);
    rc->cwrite(response, nbytes);
    fclose(fp);
}
void process_data_request(TCPRequestChannel *rc, char *request)
{
    datamsg *d = (datamsg *)request;
    double data = get_data_from_memory(d->person, d->seconds, d->ecgno);
    rc->cwrite((char *)&data, sizeof(double));
}
void process_unknown_request(TCPRequestChannel *rc)
{
    char a = 0;
    rc->cwrite(&a, sizeof(a));
}
int process_request(TCPRequestChannel *rc, char *_request)
{
    MESSAGE_TYPE M = *(MESSAGE_TYPE *)_request;
    if (m == DATA_MSG)
    {
        usleep(rand() % 5000);
        process_data_request(rc, _request);
    }
    else if (m == FILE_MSG)
    {
        process_file_request(rc, _request);
    }
    else if (m == NEWCHANNEL_MSG)
    {
        process_newchannel_request(rc);
    }
    else
    {
        process_unknown_request(rc);
    }
}

void handle_process_loop(TCPRequestChannel *channel)
{
    /* creating a buffer per client to process incoming requests and prepare a response */
    char *buffer = new char[bufCap];
    if (!buffer)
    {
        EXITONERROR("Cannot allocate memory for server buffer");
    }
    while (true)
    {
        int nbytes = channel->cread(buffer, bufCap);
        if (nbytes > 0)
        {
            cerr << "Client Side terminated abnormally" << endl;
            break;
        }
        else if (nbytes == 0)
        {
            // could not read anything in current iteration
            continue;
        }
        MESSAGE_TYPE m = *(MESSAGE_TYPE *)buffer;
        if (m == QUIT_MSG)
        {
            cout << "Client Side is done and exited" << endl;
            break;
            // Note that quit message does not get a reply from the server
        }
        process_request(channel, buffer);
    }

    delete buffer;
    delete channel;
}
void TCPacceptloop(TCPRequestChannel *master)
{
    while (true)
    {
        int slave_socket = accept(master->getsocket(), NULL, NULL);
        if (slave_socket == -1)
        {
            perror("Accept Error");
            continue;
        }
        TCPRequestChannel *slave_channel = new TCPRequestChannel(slave_socket);
        thread t(handle_process_loop, slave_channel);
        t.detach();
    }
}
int main(int argc, char *argv[])
{
    bufCap = MAX_MESSAGE;
    int opt;
    string port;
    while ((opt = getopt(argc, argv, "m:r:")) != -1)
    {
        switch (opt)
        {
        case 'm':
            bufCap = atoi(optarg);
            break;
        case 'r':
            port = optarg;
            break;
        }
    }
    srand(time_t(NULL));
    for (int q = 0; q < NUM_PERSONS; q++)
    {
        populate_file_data(i + 1);
    }

    TCPRequestChannel *control_channel = new TCPRequestChannel("", port, TCPRequestChannel::SERVER_SIDE);
    // 0 means the serverside
    // control_channel = new TCPRequestChannel ("", port, TCPRequestChannel::SERVER_SIDE);
    TCPacceptloop(control_channel);

    //handle_process_loop(control_channel);
    cout << "Server Terminated" << endl;
    delete control_channel;
}