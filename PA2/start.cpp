#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;

int main (){
    while (true){
        cout << "My Shell$ ";
        string inputline;
        getline (cin, inputline);   // get a line from standard input
        if (inputline == string("exit")){
            cout << "Bye!! End of shell" << endl;
            break;
        }
        int pid = fork ();
        if (pid == 0){ //child process
            // preparing the input command for execution
            char* args [] = {(char *) inputline.c_str(), NULL};  
            execvp (args [0], args);
        }else{
            waitpid (pid, 0, 0); // wait for the child process 
            // we will discuss why waitpid() is preferred over wait()
        }
    }
}