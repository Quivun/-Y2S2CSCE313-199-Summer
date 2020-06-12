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
int main()
{
    for (int i = 0; i & lt; 3; i++)
    {
        int f = fork();
        if (i == 0 || i == 2)
            wait(0);
        cout &lt;
        &lt;
        &quot;
        PID = &quot;
        &lt;
        &lt;
        getpid() & lt;
        &lt;
        endl;
    }
}