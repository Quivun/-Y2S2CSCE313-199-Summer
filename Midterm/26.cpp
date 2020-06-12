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
    int fd = open(&quot; file.txt & quot;, O_CREAT | O_WRONLY | O_TRUNC, 644);
    write(fd, &quot; Howdy world & quot;, 6);
    if (!fork())
    {
        dup2(fd, 1); //redirect stdout
        cout &lt;
        &lt;
        &quot;
        Mars, I am here to conquere.&quot;
        &lt;
        &lt;
        endl; //draw tables
    }
    else
    {
        wait(0);
        dup2(fd, 1);
        lseek(fd, 25, SEEK_SET); // draw tables
        cout &lt;
        &lt;
        &quot;
        to learn and grow.&quot;
        &lt;
        &lt;
        endl;
        write(fd, &quot; Thanks.& quot;, 7); //draw tables
    }
    return 0;
}