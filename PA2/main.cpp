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

vector<string> txtSplit(string inp, string divide)
{
    vector<string> ret;
    string cur = "";
    for (int q = 0; q < inp.length(); q++)
    {
        if (divide == inp.substr(q, 1))
        {
            ret.push_back(cur);
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
    while (isRoot)
    {
        cout << "ShellCMDLine$ ";
        string inputline;
        getline(cin, inputline); // get a line from standard input
        vector<string> cmdList = txtSplit(inputline, "|");
        if (inputline == string("exit"))
        {
            cout << "Shell Exiting" << endl;
            exit(0);
            // MAybe implement a global reap
        }
        else
        {
            if (cmdList.size() < 2)
            {
                // Regular input output
                cout << "Single Testing" << endl;
                int pid = fork();
                if (pid == 0)
                {
                    isRoot = false;
                    char *args[] = {(char *)cmdList[0].c_str(), NULL};
                    if (-1 == execvp(args[0], args))
                    {
                        exit(1);
                    }
                }
                else
                {
                    cout << "Parent ( " << pid << ") wait on Child." << endl;
                    waitpid(pid, 0, 0); // wait for the child process
                    cout << "Parent ( " << pid << ") has returned." << endl;
                    // we will discuss why waitpid() is preferred over wait()
                }
            }
            else
            {
                // Has piping and must now do the deed
                cout << "Multi Testing" << endl;
                int fd[2];
                pipe(fd);
                int itr = 0;
                while (itr++ != cmdList.size())
                {
                    int fd[2];
                    pipe(fd);
                    int pid = fork();
                    if (!pid)
                    {
                        isRoot = false;
                        dup2(fd[1], 1);
                        char *args[] = {(char *)cmdList[cmdList.size()-1-itr].c_str(), NULL};
                        if (-1 == execvp(args[0], args))
                        {
                            cout << "Child Error" << endl;
                            exit(1);
                        }
                    }
                    else
                    {
                        waitpid(pid, 0, 0);
                        if (isRoot)
                        {
                            char *args[] = {(char *)cmdList[cmdList.size()-1].c_str(), NULL};
                            if (-1 == execvp(args[0], args))
                            {
                                cout << "Root Error" << endl;
                                exit(1);
                            }
                        }
                        dup2(fd[0], 0);
                        close(fd[1]);
                        itr = cmdList.size();
                    }
                }
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