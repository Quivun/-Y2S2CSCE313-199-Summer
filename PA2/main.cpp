#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <string.h>

using namespace std;

void shell()
{
    /*
    bool quit = false;
    while (!quit)
    {
        cout << "Shell$ ";
        string inputline;
        getline(cin, inputline); // get a line from standard input
        if (inputline == string("exit"))
        {
            cout << "Shell Exiting" << endl;
            quit = true;
        }
        else
        {
            int pid = fork();
            if (pid == 0)
            {   
                //child process
                // preparing the input command for execution
                char *args[] = {(char *)inputline.c_str(), NULL};
                execvp(args[0], args);
            }
            else
            {
                waitpid(pid, 0, 0); // wait for the child process
                // we will discuss why waitpid() is preferred over wait()
            }
        }
    }
    */

    /*
    char buf[10];
    int fds[2];
    pipe(fds);
    if (!fork())
    {
        // on the child side
        char *msg = "a test message";
        printf("CHILD: Sent %s\n", msg);
        write(fds[1], msg, strlen(msg) + 1);
    }
    else
    {
        char buf[100];
        read(fds[0], buf, 100);
        printf("PARENT:Recv %s\n", buf);
    }
    */
    while (true)
    {
        // continue until the user enters a blank like
        string line = read a line from the user;
        // now split the line by | symbol that gives us separate commands
        // You NEED to write this function
        // for instance: ls -la | grep Jul | grep . | grep .cpp
        vector<string> c = split(line, "|");
        //after the above, levels.size() == 4, because there are
        // 3 pipe symbols making 4 pipe levels
        for (int i = 0; i < c.size(); i++)
        {
            // set up the pipe
            int fd[2];
            pipe(fd);
            int pid = fork();
            if (!pid)
            {
                // in the child process
                // 1. redirect the output to the next level
                // 2. execute the command at this level
                if (i < levels.size() - 1)
                {
                    dup2(fd[1], 1);
                    // redirect STDOUT to fd[1], so that it can write to the other side be closed
                }
            }
            else
            {
                // in the parent process
                if (i == levels.size() - 1)
                { // wait only for the last child
                    waitpid(cid);
                }
                dup2(fd[0], 0);
                // now redirect the input for the next loop iteration
                close(fd[1]);
                /*1. wait for the child process running the current level command */
                //2. redirect input from the child process
            }
        }
    }
}

int main()
{
    shell();
    return 0;
}