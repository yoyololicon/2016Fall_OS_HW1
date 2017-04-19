#include <iostream>
#include <string>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
using namespace std;

int parse(char *line, char **argv, char **pp)
{
     int p = 0;
     while (*line != '\0')
     {
		 while (*line == ' ' || *line == '\t' || *line == '\n' || *line == '|' || *line == '>' || *line == '<')
         {
			 if (*line == '|')
             {
                 *argv = '\0';
				 p = 1;
			 }
             else if(*line == '>')
             {
                 *argv = '\0';
                 p = 2;
             }
             else if(*line == '<')
             {
                 *argv = '\0';
                 p = 3;
             }
			 *line++ = '\0';
		 }
		 if (!p)
			 *argv++ = line;
		 else
			 *pp++ = line;
		 while (*line != '\0' && *line != ' ' && *line != '\t' && *line != '\n')
			 line++;
	 }
     if(p)
		*pp = '\0';
	 else
         *argv = '\0';
     return p;
}

bool insertSpace(string &cmd)
{
    for(int i = 0; i < cmd.size() ; i++)
    {
        if(cmd[i] == '>' || cmd[i] == '<' || cmd[i] == '&' || cmd[i] == '|')
        {
            if(cmd[i-1] != ' ')
            {
                cmd.resize(cmd.size()+1);
                for(int j = cmd.size()-1 ; j > i ; j--)
                    cmd[j] = cmd[j-1];
                cmd[i] = ' ';
            }
            else if(cmd[i+1] != ' ' && i != cmd.size()-1)
            {
                cmd.resize(cmd.size()+1);
                for(int j = cmd.size()-1 ; j > i+1 ; j--)
                    cmd[j] = cmd[j-1];
                cmd[i+1] = ' ';
            }
        }
    }
    if(cmd[cmd.size()-1] == '&')
    {
        cmd.resize(cmd.size()-2);
        return false;
    }
    else
        return true;
}

int main(int argc, char *argv[])
{
    pid_t pid, pid2;
    string command;
    char *terms[10], *pp[10];
    char *temp;
    bool w;
    int pipefd[2], p, fl;

    cout << ">" ;
    getline(cin, command);
    
    while(command != "quit")
    {
        w = insertSpace(command);
        temp = new char[command.size()];
        strcpy(temp, command.c_str());
        p = parse(temp, terms, pp);
	
        if(p == 1)
        {
            pipe(pipefd);

            pid = fork();
            if (pid < 0)
            {
                /* error occurred */
                cout << "Fork Failed" << endl;
                _exit(EXIT_FAILURE);
            }
            else if(pid == 0)
            {
                dup2(pipefd[0], STDIN_FILENO);
                close(pipefd[0]);
                close(pipefd[1]);
                execvp(pp[0], pp);
                cout << "command Failed" << endl;
                _exit(1);
            }
            else
            {
                /* parent process */
                /* parent will wait for the child to complete */
                pid2 = fork();
                if(pid2 < 0)
                {
                    cout << "Fork Failed" << endl;
                    _exit(EXIT_FAILURE);
                }
                else if(pid2 == 0)
                {
                    dup2(pipefd[1], STDOUT_FILENO);
                    close(pipefd[0]);
                    close(pipefd[1]);
                    execvp(terms[0], terms);
                    cout << "command Failed" << endl;
                    _exit(EXIT_FAILURE);

                }
                else
                {
                    waitpid(pid2, NULL, 0);
                    close(pipefd[0]);
                    close(pipefd[1]);
                }
                wait(NULL);
            }
        }
        else if(p == 2)
        {
            string flname(pp[0]);
            fl = open(flname.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR);
            if(!fl)
            {
                cout << "Can't open file." << endl;
                exit(EXIT_FAILURE);
            }

            pid = fork();
            if(pid < 0)
            {
                cout << "Fork Failed" << endl;
                _exit(EXIT_FAILURE);
            }
            else if(pid == 0)
            {
                dup2(fl, 1);
                close(fl);
                execvp(terms[0], terms);
                cout << "command Failed" << endl;
                _exit(EXIT_FAILURE);
            }
            else
            {
                wait(NULL);
                close(fl);
            }
        }
        else if(p == 3)
        {
            string flname(pp[0]);
            fl = open(flname.c_str(), O_RDONLY, S_IWUSR | S_IRUSR);
            if(!fl)
            {
                cout << "Can't open file." << endl;
                exit(EXIT_FAILURE);
            }

            pid = fork();
            if(pid < 0)
            {
                cout << "Fork Failed" << endl;
                _exit(EXIT_FAILURE);
            }
            else if(pid == 0)
            {
                dup2(fl, 0);
                close(fl);
                execvp(terms[0], terms);
                cout << "command Failed" << endl;
                _exit(EXIT_FAILURE);
            }
            else
            {
                wait(NULL);
                close(fl);
            }
        }
        else
        {
            pid = fork();
            if (pid < 0)
            {
                /* error occurred */
                cout << "Fork Failed" << endl;
                _exit(EXIT_FAILURE);
            }
            else if(!pid && w)
            {
                /* child process B */
                pid = execvp(terms[0], terms);
                if(pid < 0)
                {
                    cout << "command Failed" << endl;
                    _exit(EXIT_FAILURE);
                }
                _exit(0);
            }
            else if (pid == 0)
            {
                /* child process C */
                if(pid = fork())
                    _exit(0);
                else if(pid == 0)
                {
                    pid = execvp(terms[0], terms);
                    if(pid < 0)
                    {
                        cout << "command Failed" << endl;
                        _exit(EXIT_FAILURE);
                    }
                    _exit(0);
                }
                else
                {
                    cout << "Fork Failed" << endl;
                    _exit(EXIT_FAILURE);
                }
            }
            else
            {
                /* parent process */
                /* parent will wait for the child to complete */
                wait(NULL);
            }
        }
        delete [] temp;
        w = false;
        cout << ">" ;
        getline(cin, command);
    }
    return 0;
}
