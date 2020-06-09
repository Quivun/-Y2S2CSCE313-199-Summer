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
}

int main()
{
    shell();
    return 0;
}