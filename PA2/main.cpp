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
    while (true)
    {
        cout << "ShellCMDLine$ ";
        string inputline;
        if (inputline == string("exit"))
        {
            cout << "Shell Exiting" << endl;
            break;
        }
        else
        {
            getline(cin, inputline); // get a line from standard input
            vector<string> cmdList = txtSplit(inputline, "|");
            int pid = fork();
            if (cmdList.size() < 2)
            {
                // Regular input output
                int pid = fork();
                if (pid == 0)
                {
                    char *args[] = {(char *)inputline.c_str(), NULL};
                    execvp(args[0], args);
                }
                else
                {
                    waitpid(pid, 0, 0); // wait for the child process
                    // we will discuss why waitpid() is preferred over wait()
                }
            }
            else
            {
                // Has piping and must now do the deed
                for (int q = 0; q < cmdList.size(); q++)
                {
                    int fd[2];
                    pipe(fd);
                    int pid = fork();
                    if (!pid) // Child
                    {
                        if (q < cmdList.size() - 1)
                        {
                            dup2(fd[1], 1); // redirect STDOUT to fd[1], so that it can write to the other side be closed
                        }
                        char *args[] = {(char *)cmdList[q].c_str(), NULL};
                        execvp(args[0], args);
                    
                    }
                    else
                    { // Parent
                        if (q == cmdList.size() - 1)
                        {
                            waitpid(pid,0,0);
                        }
                        dup2(fd[0], 0); // now redirect the input for the next loop iteration
                        close(fd[1]);
                        /*1. wait for the child process running the current level command */
                        //2. redirect input from the child process
                    }
                }
            }
        }
    }
}

    int main()
    {
        shell();
        return 0;
    }