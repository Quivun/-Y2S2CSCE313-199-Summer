#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <string.h>
#include <vector>
using namespace std;

int maxLen = sysconf(_SC_ARG_MAX);

pair<int, string> msgHandler(string inp, int ind)
{
    pair<int, string> ret = {ind, ""};
    string div = inp.substr(ind++, 1);
    for (int q = ind; q < inp.length(); q++)
    {
        if (inp.substr(q, 1) == div)
        {
            ret.first = q;
        }
        else
        {
            ret.second += inp.substr(q, 1);
        }
    }
    return ret;
}

vector<string> txtSplit(string inp, string divide)
{
    vector<string> ret;
    string cur = "";
    for (int q = 0; q < inp.length(); q++)
    {
        if (inp.substr(q, 1) == "\"" || inp.substr(q, 1) == "\'")
        {
            pair<int, string> dQ = msgHandler(inp, q);
            if (divide == " ")
            {
                ret.push_back(dQ.second);
            }
            else
            {
                cur += dQ.second;
            }
            q = dQ.first;
        }
        else if (divide == inp.substr(q, 1))
        {
            if (cur.length() > 0)
            {
                ret.push_back(cur);
            }
            cur = "";
        }
        else
        {
            cur += inp.substr(q, 1);
        }
    }
    if (cur.length() != 0)
    {
        ret.push_back(cur);
    }
    return ret;
}

void shell()
{
    bool isRoot = true;
    int std_in = dup(0);
    int std_out = dup(1);
    while (isRoot)
    {
        bool bg = false;
        cout << "ShellCMDLine$ ";
        string inputline;
        getline(cin, inputline); // get a line from standard input
        vector<string> cmdListSplitPipe = txtSplit(inputline, "|");
        vector<vector<string>> cmdList;
        for (int q = 0; q < cmdListSplitPipe.size(); q++)
        {
            cmdList.push_back(txtSplit(cmdListSplitPipe[q], " "));
        }
        if (cmdList[cmdList.size() - 1][cmdList[cmdList.size() - 1].size() - 1] == "&")
        {
            bg = true;
            cmdList[cmdList.size() - 1].pop_back();
        }
        if (inputline == string("exit"))
        {
            // cout << "Shell Exiting" << endl;
            exit(0);
            // Maybe implement a global reap
        }
        else if (cmdList[0][0] == "cd")
        {
            if (cmdList[0].size() == 1)
            {
                // cout << "It should be changed" << endl;
                chdir("/home/osboxes");
            }
            else if (cmdList[0][1] == "-")
            {
                chdir("..");
            }
            {
                // cout << "This is for specifics " << endl;
                const char *change = cmdList[0][1].c_str();
                chdir(change);
            }
        }
        else
        {
            if (cmdList.size() < 2)
            {
                // Regular input output
                // cout << "Single Testing" << endl;
                int pid = fork();
                if (pid == 0)
                {
                    isRoot = false;
                    /*
                    char* args[maxLen];                
                    for (int q = 0; q < cmdList[0].size(); q++){
                        strcpy(args[q], cmdList[0][q].c_str());
                    }
                    strcpy(args[cmdList[0].size()], NULL);
                    if (-1 == execvp(args[0], args))
                    {
                        exit(1);
                    }
                    */

                    char **args = (char **)malloc((cmdList[0].size()) * sizeof(char *));
                    for (int q = 0; q < cmdList[0].size(); q++)
                    {
                        args[q] = (char *)cmdList[0][q].c_str();
                    }
                    args[cmdList[0].size()] = (char *)NULL;
                    if (execvp(args[0], args) == -1)
                    {
                        exit(1);
                    }
                }
                else
                {
                    // cout << "Parent ( " << pid << ") wait on Child." << endl;
                    if (!bg){
                        waitpid(pid, 0, 0); // wait for the child process
                    }
                    // cout << "Parent ( " << pid << ") has returned." << endl;
                    // we will discuss why waitpid() is preferred over wait()
                }
            }
            else
            {
                // Has piping and must now do the deed
                /*
                // cout << "Multi Testing" << endl;
                int fd[2];
                pipe(fd);
                int itr = 0;
                if (!fork()){
                    dup2(std_out,1);
                    char** args[] = new char*[cmdList].size()]
                }
                */
                /*
                while (itr++ != cmdList.size())
                {
                    int fd[2];
                    pipe(fd);
                    int pid = fork();
                    if (!pid)
                    {
                        isRoot = false;
                        dup2(fd[1], 1);
                        // cout << "Child Start" << endl;
                        char *args[] = {(char *)cmdList[cmdList.size()-1-itr].c_str(), NULL};
                        if (-1 == execvp(args[0], args))
                        {
                            // cout << "Child Error" << endl;
                            exit(1);
                        }
                    }
                    else
                    {
                        waitpid(pid, 0, 0);
                        if (isRoot)
                        {
                            // cout << "Root Start" << endl;
                            // cout << cmdList[cmdList.size()-1] << endl;
                            char *args[] = {(char *)cmdList[cmdList.size()-1].c_str(), NULL};
                            if (-1 == execvp(args[0], args))
                            {
                                // cout << "Root Error" << endl;
                                exit(1);
                            }
                        } else {
                        dup2(fd[0], 0);
                        close(fd[1]);
                        itr = cmdList.size();
                        }
                    }
                }
                */
                // If pid, then continue the call list, otherwise wait for the others.
            }
        }
    }
}

int main()
{
    shell();
    return 0;
}