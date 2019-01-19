#include<sys/msg.h>     // msqid_ds, msgget(), msgctl(), msgrcv(), IPC_RMID
#include<stdio.h>       // perror(), printf(),
#include<string.h>      // memset(), strlen(), strcmp()
#include<sys/stat.h>    // stat, stat()
#include<time.h>        // timespec_get(),  strftime(), TIME_UTC, localtime()
#include<unistd.h>      // chdir()
#include<stdlib.h>      // exit()
#include<sys/ipc.h>

#ifndef MSG
#define MSG 1
#endif
#ifndef MSGMAX
#define MSGMAX 512
#endif

/*
 * +Выбрать из очереди сообщений последнее сообщение.
 * + Отфильтровать в нем те файлы, последняя модификация которых производилась
 *      в течение текущего дня и записать их в стандартный файл вывода.
 * + Вывести также величину общего количества байтов во всех сообщениях очереди.
 * + Удалить очередь сообщений.
 */

struct msgbuf {
    long    type;
    char    text[512];
};

int main()
{
    int msgid, i = -1, counter = 0;
    struct msqid_ds qstatus;        // status of msg queue
    key_t key=10;                   // key of msg queue
    struct msgbuf msg;              // msg
    char link[512] = "";                 // files' link
    struct stat sf;                 // files' params
    struct timespec time;           // real date
    char date_t[10], date_l[10];    // strings to compare dates ^

    // get real date
    timespec_get(&time, TIME_UTC);
    memset(date_t,0,strlen(date_t) * sizeof(char));
    strftime(date_t, sizeof date_t, "%D", gmtime(&time.tv_sec));

    // opening of msg queue
    msgid = msgget(key, 0);

    while(msgid == -1 && counter < 10){
        msgid = msgget(key, 0);
        ++counter;
        sleep(1);
    }

    if(counter == 10) {
        printf("error. I dont want to wait anymore\n");
        exit(1);
    }

    counter = 0;
    // getting of parameters of queue
     do{
        if(msgctl(msgid,IPC_STAT, &qstatus)<0){
            perror("msgctl failed");
            msgctl(msgid, IPC_RMID, NULL);
            exit(1);
        }
        sleep(1);
        ++counter;
    }while(qstatus.msg_qnum == 0 && counter < 10);

    if(counter == 10) {
        printf("error. I dont want to wait msg anymore\n");
        exit(1);
    }

    printf("Current number of bytes on queue %lu\n",qstatus.msg_cbytes);

    // receiving the last message
    while(qstatus.msg_qnum > 0)
        if( msgrcv(msgid, &msg, MSGMAX, MSG, IPC_NOWAIT | MSG_NOERROR) != -1 ){
            msgctl(msgid,IPC_STAT, &qstatus);
        }else{
            perror("msgrcv error\n");
            msgctl(msgid, IPC_RMID, NULL);
            exit(1);
        }
    //printf("%s\n",msg.text);
    counter = 0;

    for(int p = 0; p < strlen(msg.text); ++p){
                if(msg.text[p] == '\n'){

                    // getting files' modification date
                    if(stat(link, &sf) < 0) perror("stat error\n");
                    else {
                        strftime(date_l, 20, "%m/%d/%y", localtime(&(sf.st_mtime)));
                        // comparing dates ( real and files')
                        if(strcmp(date_l,date_t) == 0){
                            printf("%s\n", link);
                            ++counter;
                        }
                    }
                    i = -1;
                    memset(link,0,strlen(link) * sizeof(char));

                }else{
                        ++i;
                        link[i] = msg.text[p];
                }
    }
    if(counter == 0) printf("no such files in spool\n");

    // deleting of msg queue
    msgctl(msgid, IPC_RMID, NULL);
}
