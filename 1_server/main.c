#include<stdlib.h>      // exit(), system()
#include<sys/wait.h>    // wait()
#include<sys/msg.h>     // msqid_ds, msgget(), msgctl(), msgrcv(), msgsnd()
#include<stdio.h>       // perror(), printf(),
#include<string.h>      // memset(), strlen()
#include<unistd.h>      // chdir()
#include<sys/ipc.h>


#ifndef MSG
#define MSG 1
#endif
#ifndef MSGMAX
#define MSGMAX 512
#endif

// Создать очередь сообщений. Записать в неё сообщение об именах файлов,
// помещенных в спулинг.

struct msgbuf {
    long    type;
    char    text[512];
};

int main()
{
    int msgid, s;
    struct msgbuf msg;          // msg
    int fd[2];                  // pipe
    key_t key=10;               // key of msg queue

    memset(msg.text,0,strlen(msg.text) * sizeof(char));
    msg.type = MSG;

    // pipe
    if(pipe(fd) == -1){
        printf("Error\n");
        return 0;
    }

    msgid=msgget(key, IPC_EXCL | IPC_CREAT | 0666);

    if(msgid==-1){
        perror("msgget failed");
        exit(1);
    } else printf("msgget: msgget succeeded: msqid = %d\n", msgid);

    // redirection of output of command to msgs' text
    // command outputs names of all files in spool
    if(fork()== 0){
        close(1);
        close(fd[0]);
        dup2(fd[1],1);
        close(fd[1]);
        system("find /home/anna/Desktop -type f -print 2>/dev/null");
        exit(0);
    }else{
        wait(&s);
        close(fd[1]);
        while (read(fd[0], msg.text, MSGMAX) != 0)
            msgsnd(msgid, &msg, strlen(msg.text), 0);
    }

}
