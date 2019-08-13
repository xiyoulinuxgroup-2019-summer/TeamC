#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <string.h>
#include <linux/socket.h>
#include <string.h>
#include <pthread.h>

#include "chat.h"

int fd;  //用来通信的文件描述符
char myname[20]; //自己的名字

pthread_mutex_t mutex;

void my_err(char *err, int line){  //错误处理函数
    printf("line: %d", line);
    perror(err);
    exit(1);
}

void *recv_infor(void *arg);
void *send_request(void *arg);


int login_face(int conn_fd);            //登录界面
int user_face(int conn_fd);             //conn_fd  //用户界面
int reg(int conn_fd);                   //注册功能
int login(int conn_fd);                 //登录功能
int my_friend(int conn_fd);             //我的好友功能
void *server_message(void *arg);        //线程处理函数
int choice_group(int conn_fd);          //选择群聊
int deal_shenqing(int conn_fd);         //处理申请
int add_friend(int conn_fd);            //添加好友
int create_group(int conn_fd);          //创建群聊
int exit_face(int conn_fd);             //注销
int all_friends(int conn_fd);           //查看所有好友
int online_friends(int conn_fd);        //查看在线好友

int main(int argc, char * argv[])
{
    if(argc < 2)
    {
        printf("eg: ./a.out port\n");
        exit(1);
    }
    pthread_mutex_init(&mutex, NULL);
    int ret;
    pthread_t pthread1;
    int port = atoi(argv[1]); //字符串变量转换成整形变量
    //创建套接字
    fd = socket(AF_INET, SOCK_STREAM, 0);

    //连接服务器
    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
    //serv.sin_addr.s_addr = htonl()
    inet_pton(AF_INET, "127.0.0.1", &serv.sin_addr.s_addr);
    ret = connect(fd, (struct sockaddr*)&serv, sizeof(serv) );
    if(ret == -1){
        perror("connect");
        printf("line: %d\n", __LINE__-3);
        exit(1);
    }

    //通信
    //显示登录界面
    while(1){
        char choice[10]; //登录界面的选择
        printf("---------------------\n");
        printf("       1.注册\n");
        printf("       2.登录\n");
        printf("       3.退出\n");
        printf("---------------------\n");

        while(1){
            printf("请输入你的选择：");
            scanf("%s", choice);
            
            if(strcmp(choice, "1")==0 || strcmp(choice, "2")==0 || strcmp(choice, "3")==0){
                break;
            }
            else{
                printf("输入错误，请按照提示输入\n");
            }
        }

        if(choice[0] == '3'){  //退出
            close(fd);  //断开连接
            exit(1);
        }
        
        if(choice[0] == '1'){  //选择注册
            ret = reg(fd);
            if(ret == 1){ //注册成功
                //break;
                continue;
            }
            else{
                continue;
            }
        }
        else if(choice[0] == '2'){  //选择登录
            ret = login(fd);
            if(ret == 1){  //登陆成功
                break;
            }
            else{          //登录失败
                break;
            }
        }
    }

    //登陆成功，创建用户线程，接受服务器的消息
    pthread_create(&pthread1, NULL, server_message, NULL);
    pthread_detach(pthread1);
    user_face(fd);

    return 0;
}

//线程处理函数
void *server_message(void *arg){
    while(1){
        MSG message;
        char *recv_message;
        recv_message = (char*)malloc(sizeof(message)); //每次接受信息的为一个结构体大小
        memset(&message, 0, sizeof(message));
    
        int len = recv(fd, recv_message, sizeof(message), 0);
        if(len == -1){
            my_err("recv", __LINE__-2);
        }
        else if(len == 0){
            printf("服务器断开连接\n");
            pthread_exit(0);
        }
        memcpy(&message, recv_message, sizeof(message));
        //printf("----message.cmd: %d\n", message.cmd);
        switch(message.type){
        /*case 'a':
            printf("\n");
            if(message.friend_num == 0){
                printf("你还没有好友\n");
            }
            if(message.friend_num){
                printf("好友：%s\n", message.friend_name);
            }
            break;*/
        }
    }
}
int user_face(int conn_fd) {
    char choice[2];
    while(1){
        printf("-----------------------\n");
        printf("        1.我的好友\n");
        printf("        2.选择群聊\n");
        printf("        3.处理申请\n");
        printf("        4.添加好友\n");
        printf("        5.创建群聊\n");
        printf("          6.注销\n");
        printf("-----------------------\n");
        //printf("    请输入你的选择：");

        //scanf("%s", ch);
        //while(getchar() != '\n');
    
        while(1){
            printf("   请输入你的选择：");
            scanf("%s", choice);

            if(strcmp(choice, "1")==0||strcmp(choice, "2")==0||strcmp(choice, "3")==0||strcmp(choice, "4")==0||
                strcmp(choice, "5")==0||strcmp(choice, "6")==0){
                break;
            }
            else{
                printf("输入错误，请按照提示输入\n");
            }
        }

        switch(choice[0]){
        case '1':
            my_friend(conn_fd);
            break;
        /*case '2':
            choice_group(conn_fd);
            break;
        case '3':
            deal_shenqing(conn_fd);
            break;
        case '4':
            add_friend(conn_fd);
            break;
        case '5':
            create_group(conn_fd);
            break;*/
        case '6':
            //exit_face(conn_fd);
            close(conn_fd);
            exit(1);
            break;
        }
    }

    return 0;
}

int reg(int conn_fd) {
    MSG message;
    message.type = 'r';

    char buf[256];
    printf("*姓名：");
    scanf("%s", buf);
    while(getchar() != '\n');
    strcpy(message.user_infor.name, buf);

    printf("昵称：");
    scanf("%s", buf);
    while(getchar() != '\n');
    strcpy(message.user_infor.nickname, buf);

    printf("性别：");
    scanf("%s", buf);
    while(getchar() != '\n');
    strcpy(message.user_infor.sex, buf);

    printf("密码：");
    scanf("%s", buf);
    while(getchar() != '\n');
    strcpy(message.user_infor.password, buf);

    send(conn_fd, &message, sizeof(message), 0);
    recv(conn_fd, &message, sizeof(message), 0);

    printf("cmd = %d\n", message.cmd);
    //注册成功返回 1001
    if(message.cmd == 1001) {
        printf("注册成功\n");
        printf("正在返回登录界面\n");
        sleep(1);
        return 1;
    }
    //注册失败返回 -1
    else if(message.cmd == -1) {
        printf("该用户已被注册\n");
        printf("正在返回登录界面\n");
        return -1;
    }
}

int login(int conn_fd){
    MSG message;
    memset(&message, 0, sizeof(message));
    message.type = 'l';

    char buf[256];
    printf("用户名：");
    scanf("%s", buf);
    while(getchar() != '\n');
    strcpy(message.user_infor.name, buf);
    strcpy(myname, buf);

    printf("密  码：");
    scanf("%s", buf);
    while(getchar() != '\n');
    strcpy(message.user_infor.password, buf);
    
    send(conn_fd, &message, sizeof(message), 0);
    recv(conn_fd, &message, sizeof(message), 0);

    //登录成功返回1002
    //printf("cmd = %d\n", message.cmd);
    if(message.cmd == 1002) {
        printf("登陆成功\n");
        sleep(1);
        return 1;     
    }
    //登录失败返回-2
    else if(message.cmd == -2) {
        printf("登录失败\n");
        sleep(1);
        return -1;
    }
}

int my_friend(int conn_fd){
    char choice[10];
    
    while(1){
        memset(choice, 0, sizeof(choice));
        printf("-------------------------\n");
        printf("     1.查看所有好友\n");
        printf("     2.查看在线好友\n");
        printf("     3.选择私聊\n");
        printf("     4.添加好友\n");
        printf("     5.删除好友\n");
        printf("     0.返回上级菜单\n");
        printf("-------------------------\n");

        while(1){
            printf("请输入你的选择：");
            scanf("%s", choice);
            while(getchar() != '\n')
                continue;

            if(strcmp(choice, "1")==0||strcmp(choice, "2")==0||strcmp(choice, "3")==0||strcmp(choice, "4")==0||strcmp(choice, "0")==0){
                break;
            }
            else{
                printf("错误输入，请按照提示输入\n");
            }
        }

        switch(choice[0]){
        case '0':
            return 0;
        case '1':
            all_friends(conn_fd);
            break;
        case '2':
            online_friends(conn_fd);
            break;
        }
    }
}

int all_friends(int conn_fd){
    MSG message;
    int len = sizeof(message);
    memset(&message, 0, len);
    message.type = 'a';
    
    strcpy(message.user_infor.name, myname);

    if(send(conn_fd, &message, sizeof(message), 0) != len){
        my_err("send", __LINE__-1);
    }
    recv(conn_fd, &message, sizeof(message), 0);
    if(message.friend_num == 0){
        printf("你还没有朋友\n");
    }
    else{
        for(int i=0; i<message.friend_num; i++){
            recv(conn_fd, &message, sizeof(message), 0);
            printf("haoyou: %s\n", message.friend_name);
        }
    }
    printf("按任意键返回菜单\n");
    getchar();
}

int online_friends(int conn_fd){
    MSG message;
    int len = sizeof(message);
    memset(&message, 0, len);
    
    message.type = 'o';
    strcpy(message.user_infor.name, myname);
    if(send(conn_fd, &message, sizeof(message), 0) != len){
        my_err("send", __LINE__-1);
    }
    int ret = recv(conn_fd, &message, sizeof(message), 0);
    if(ret == -1){
        my_err("recv", __LINE__-2);
    }
    if(message.friend_num == 0){
        printf("目前还没有还有在线\n");
    }
    else{
        printf("在线好友：\n");
        for(int i=0; i<message.friend_num; i++){
           recv(conn_fd, &message, sizeof(message), 0);
           printf("%s\n", message.friend_name);
        }
    }
    printf("按任意键返回菜单");
    getchar();
}
