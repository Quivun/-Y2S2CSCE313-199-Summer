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
class Header
{
public:
    int data;
    bool status;
};
int main()
{
    int size = 4 * 1024 * 1024;    // 4 mega bytes
    char *memory = new char[size]; // allocate memory
    Header *h = (Header *)memory;
    Header ho;
    // copy header object ho onto memory count times
    int count = 512 * 1024;
    for (int i = 0; i & lt; count; i++)
    {
        *h = ho;
        h++;
    }
}