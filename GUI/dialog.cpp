/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:

**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtWidgets>
#include "dialog.h"
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <curses.h>
#include <pthread.h>
#include <QInputDialog>
#include <QTimer>
#include <QString>
#include <sstream>
#include <string.h>
using namespace std;

char * user;
char * password;
char * pwd;
char * host = "localhost";
char * sport;
char * args;
char * room;
char * username;
int port = 8081;
//self
//char * temp_user;
//char * temp_pwd;
//char * temp_room;
char * room_name = "";

#define MAX_MESSAGES 100
#define MAX_MESSAGE_LEN 300
#define MAX_RESPONSE (20 * 1024)

int lastMessage = 0;
bool limitUsers=false;

int open_client_socket(char * host, int port) {
        // Initialize socket address structure
        struct  sockaddr_in socketAddress;

        // Clear sockaddr structure
        memset((char *)&socketAddress,0,sizeof(socketAddress));

        // Set family to Internet
        socketAddress.sin_family = AF_INET;

        // Set port
        socketAddress.sin_port = htons((u_short)port);

        // Get host table entry for this host
        struct  hostent  *ptrh = gethostbyname(host);

        if ( ptrh == NULL ) {
                perror("gethostbyname");
                exit(1);
        }

        // Copy the host ip address to socket address structure
        memcpy(&socketAddress.sin_addr, ptrh->h_addr, ptrh->h_length);

        // Get TCP transport protocol entry
        struct  protoent *ptrp = getprotobyname("tcp");
        if ( ptrp == NULL ) {
                perror("getprotobyname");
                exit(1);
        }

        // Create a tcp socket
        int sock = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
        if (sock < 0) {
                perror("socket");
                exit(1);
        }

        // Connect the socket to the specified server
        if (connect(sock, (struct sockaddr *)&socketAddress,
                    sizeof(socketAddress)) < 0) {
                perror("connect");
                exit(1);
        }
        return sock;
}

int sendCommand(char * host, int port, char * command, char * user,
      char * password, char * args, char * response) {
      int sock = open_client_socket( host, port);

      // Send command
      write(sock, command, strlen(command));
      write(sock, " ", 1);
      write(sock, user, strlen(user));
      write(sock, " ", 1);
      write(sock, password, strlen(password));
      write(sock, " ", 1);
      write(sock, args, strlen(args));
      /*if(strlen(args2)!=0) {
          write (sock, " ", 1);
          write (sock, args2, strlen(args2));
      }
      */
      write(sock, "\r\n",2);

     // Keep reading until connection is closed or MAX_REPONSE
     int n = 0;
     int len = 0;
     while ((n=read(sock, response+len, MAX_RESPONSE - len))>0) {
        len += n;
     }
    response[len]='\0';
    //printf("response:%s\n", response);

    close(sock);
}

void get_rooms(){
    char response[ MAX_RESPONSE ];
    sendCommand(host, port, "GET-ALL-ROOMS", user, password, "", response);

    if (!strcmp(response,"OK\r\n")) {
            //printf("Rooms: %s\n", response);
    }
}

void enter_room(char *room) {
    char response[ MAX_RESPONSE ];
    sendCommand(host, port, "ENTER-ROOM", user, password, room, response);

    if (!strcmp(response,"OK\r\n")) {
            printf("User %s has entered room %s\n", user, room);
    }

    //void = changing the response and use a global variable response
    //char = return response
    //which one is better?
}

void leave_room(char *room) {
    char response[ MAX_RESPONSE ];
    sendCommand(host, port, "LEAVE-ROOM", user, password, room, response);

    if (!strcmp(response,"OK\r\n")) {
            printf("User %s has left room %s\n", user, room);
    }
}

void get_messages(char * room) {
    char response[ MAX_RESPONSE ];
    sendCommand(host, port, "ENTER-ROOM", user, password, args, response);

    if (!strcmp(response,"OK\r\n")) {
            printf("User %s has entered room %s\n", user, room);
    }
}

void send_message(char * room, char * msg) {
    char response[ MAX_RESPONSE ];
    sendCommand(host, port, "SEND-MESSAGE", user, password, args, response);

    if (!strcmp(response,"OK\r\n")) {
            printf("User %s has entered room %s\n", user, room);
    }
}

void print_users_in_room(char * room) {
    char response[ MAX_RESPONSE ];
    sendCommand(host, port, "GET-USERS-IN-ROOM", user, password, room, response);

    if (!strcmp(response,"OK\r\n")) {
            printf("User %s has entered room %s\n", user, room);
    }
}

void print_users() {
    char response[ MAX_RESPONSE ];
    sendCommand(host, port, "GET-ALL-USERS", user, password, room, response);

    if (!strcmp(response,"OK\r\n")) {
            printf("Users: ", response);
    }
}

void printHelp(){
       printf("Commands:\n");
       printf(" -who   - Gets users in room\n");
       printf(" -users - Prints all registered users\n");
       printf(" -help  - Prints this help\n");
       printf(" -quit  - Leaves the room\n");
       printf("Anything that does not start with \"-\" will be a message to the chat room\n");
}

void Dialog::sendAction()
{
    printf("Send Button\n");

    char* msginput = inputMessage->toPlainText().toLatin1().data();

    string main_str(msginput);
    string roomname(room_name);
    string msgStr = roomname + " " + main_str;
    char * msg = strdup(msgStr.c_str());

    //user = (char*) username.c_str();
    //password = (char *) pwd.c_str();

    char *resp = (char*)malloc(MAX_RESPONSE*sizeof(char));
    sendCommand(host, port,"SEND-MESSAGE", username, pwd, msg, resp);
    printf("%s", resp);
    sendCommand(host, port,"GET-MESSAGES", username, pwd ,strdup((to_string(0) + " " + room_name).c_str()),resp);
    printf("%s", resp);
    messageCount++;

    QString Qresponse = QString::fromStdString(resp);
    QStringList msgs = Qresponse.split("\r\n", QString::SkipEmptyParts);
    allMessages->clear();
    for (int i = 0; i < msgs.size(); i++){
        allMessages->append(msgs.at(i));
     }
}

void Dialog::newUserAction()
{
    printf("New User Button\n");

    QDialog * d = new QDialog();
    QVBoxLayout * vbox = new QVBoxLayout();

    d->setWindowTitle ("Create Account Page");
    QLabel * user_label = new QLabel("Username: ");
    QLineEdit * user_line = new QLineEdit();

    QLabel * pwd_label = new QLabel("Password: ");
    QLineEdit * pwd_line = new QLineEdit;
    //pwd_line->setEchoMode(QLineEdit::Password);
    QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                                        | QDialogButtonBox::Cancel);
    QObject::connect(buttonBox, SIGNAL(accepted()), d, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), d, SLOT(reject()));

    vbox->addWidget(user_label);
    vbox->addWidget(user_line);
    vbox->addWidget(pwd_label);
    vbox->addWidget(pwd_line);
    vbox->addWidget(buttonBox);

    d->setLayout(vbox);

    int result = d->exec();
    if(result == QDialog::Accepted)
    {
        // handle values from d
        qDebug() << "The user clicked:"
                 << "Username: " << user_line->text()
                 << "Password: " << pwd_line->text();
    }

    string temp_user = user_line->text().toStdString();
    string temp_pwd = pwd_line->text().toStdString();
    username = strdup(temp_user.c_str());
    pwd = strdup(temp_pwd.c_str());
    user = strdup(temp_user.c_str());
    password = strdup(temp_pwd.c_str());
    //string windowName = "CS240 IRC Client. Currently logged in as: " + temp_user +".";
    string windowName = "CS240 IRC Client. Please log in.";
    setWindowTitle(tr((char*)windowName.c_str()));
    add_user();
}

void Dialog::add_user(){
    char resp_user[MAX_RESPONSE];
    //user = (char *) username.c_str();
    //password = (char *) pwd.c_str();
    sendCommand(host, port, "ADD-USER", user, password, "", resp_user);
    usersList->addItem(user);
}

void Dialog::loginAction(){
    printf("Login Button\n");
    QDialog * d = new QDialog();
    QVBoxLayout * vbox = new QVBoxLayout();

    d->setWindowTitle ("Login Page");
    QLabel * user_label = new QLabel("Username: ");
    QLineEdit * user_line = new QLineEdit();

    QLabel * pwd_label = new QLabel("Password: ");
    QLineEdit * pwd_line = new QLineEdit();
    pwd_line->setEchoMode(QLineEdit::Password);
    QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                                        | QDialogButtonBox::Cancel);

    QObject::connect(buttonBox, SIGNAL(accepted()), d, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), d, SLOT(reject()));

    vbox->addWidget(user_label);
    vbox->addWidget(user_line);
    vbox->addWidget(pwd_label);
    vbox->addWidget(pwd_line);
    vbox->addWidget(buttonBox);

    d->setLayout(vbox);

    int result = d->exec();
    if(result == QDialog::Accepted)
    {
        // handle values from d
        qDebug() << "The user clicked:"
                 << "Username: " << user_line->text()
                 << "Password: " << pwd_line->text();
    }

    string temp_user = user_line->text().toStdString();
    string temp_pwd = pwd_line->text().toStdString();
    username = strdup(temp_user.c_str());
    pwd = strdup(temp_pwd.c_str());
    user = strdup(temp_user.c_str());
    password = strdup(temp_pwd.c_str());


    char * resp = (char*) malloc (MAX_RESPONSE * sizeof(char));
    sendCommand(host, port, "GET-ALL-ROOMS", username, pwd, "", resp);
    QString Qresponse = QString::fromStdString(resp);
    if(Qresponse.contains("ERROR")){
       QMessageBox::about(this, "Failure", "LogIn credentials invalid");
    } else {
        string windowName = "CS240 IRC Client. Currently logged in as: " + temp_user +".";
        setWindowTitle(tr((char*)windowName.c_str()));
    }

}

void Dialog::createRoomAction(){
    /*
     * QDialog * d = new QDialog();
    QVBoxLayout * vbox = new QVBoxLayout();

    d->setWindowTitle ("Login Page");
    QLabel * room_label = new QLabel("Username: ");
    QLineEdit * room_line = new QLineEdit();
    QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                                        | QDialogButtonBox::Cancel);

    QObject::connect(buttonBox, SIGNAL(accepted()), d, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), d, SLOT(reject()));

    vbox->addWidget(room_label);
    vbox->addWidget(room_line);
    vbox->addWidget(buttonBox);

    d->setLayout(vbox);

    int result = d->exec();
    if(result == QDialog::Accepted)
    {
        // handle values from d
        qDebug() << "The user clicked:"
                 << "Created Room: " << room_line->text();
    }

    if(result == QDialog::Accepted)
    {
        string temp_room = room_line->text().toStdString();
        QByteArray Qroom_arr = room_line->text().toLocal8Bit();
        temp_room = Qroom_arr.data();

        //room = (char*)temp_room.c_str();
        room = strdup(Qroom_arr);
        create_room();
    }*/

    bool ok;
    QString Qroom = QInputDialog::getText(this, "Create Room", "Enter room name",QLineEdit::Normal, NULL, &ok);

    QByteArray room_arr = Qroom.toLocal8Bit();
    room = strdup(room_arr.data());

    if(ok){
        create_room();
    }
}

void Dialog::create_room(){
    char * resp = (char *) malloc (sizeof(char)*MAX_RESPONSE);
    sendCommand(host, port, "CREATE-ROOM", username, pwd, room, resp);
    //sendCommand(host, port, "CREATE-ROOM", "admin", "admin", room, resp);
    //string respons = resp.c_str();
    //resp[respons.length()] = '\0';
    if (resp[0] == 'O' && resp[1] == 'K'){
       if(!strcmp(resp,"OK\r\n")){
            char room_arr[50];
            sprintf(room_arr, "%s", room);
            roomsList->addItem(room_arr);
        }
    }
/*
    char response[MAX_RESPONSE];
    sendCommand(host, port, "CREATE-ROOM", username, pwd, room, response);
    if(!strcmp(response, "OK\r\n")){
        printf("User %s created the room %s\n", user, room);
        roomsList->addItem(room);
    }

   */
}

void Dialog::enterRoomAction(){
    if(roomsList->currentItem() == NULL)return;
    QString Qroom_name = roomsList->currentItem()->text();
    QByteArray room_arr = Qroom_name.toLocal8Bit();
    room = strdup(room_arr.data());
    enter_room();
}

void Dialog::enter_room(){
    char * resp = (char *) malloc (sizeof(char)*MAX_RESPONSE);
    sendCommand(host, port, "ENTER-ROOM", username, pwd, room, resp);
    if (resp[0] == 'O' && resp[1] == 'K'){
        room_name = strdup(room);
        limitUsers=true;
    }
    string roomname(room_name);
    string user_name(username);
    string temp_msg =  " has entered room: ";
    string tot = temp_msg + roomname + ".";

    string msgstr = roomname + " " + tot;
    char * msg = strdup(msgstr.c_str());
    sendCommand(host, port, "SEND-MESSAGE", username, pwd, msg, resp);
    printf("%s", resp);

    room_name = strdup(room);
}

void Dialog::leaveRoomAction(){
    char * resp = (char *) malloc (sizeof(char)*MAX_RESPONSE);
    string roomname(room_name);
    string user_name(username);
    string temp_msg =  " has left room: ";
    string tot = temp_msg + roomname + ".";

    string msgstr = roomname + " " + tot;
    char * msg = strdup(msgstr.c_str());
    sendCommand(host, port, "SEND-MESSAGE", username, pwd, msg, resp);
    if (resp[0] == 'O' && resp[1] == 'K'){
        room_name = "";
        allMessages->clear();
        usersList->clear();
        limitUsers=false;
    } else {
        QMessageBox::about(this, "Failure to leave the room", "You are not in a room");
        return;
    }
    printf("%s", resp);
    sendCommand(host, port, "LEAVE-ROOM", username, pwd, room, resp);
    if (resp[0] == 'O' && resp[1] == 'K'){
        room_name = "";
        allMessages->clear();
        usersList->clear();
    } else {
        QMessageBox::about(this, "Failure to leave the room", "You are not in a room");
    }
}

void Dialog::timerAction()
{
    refreshUsersInRoom();
    refreshRoom();
    printf("Timer wakeup\n");
    messageCount++;

    if (room_name == "") {
        refreshRoom();
        return;
    }
    refreshRoom();
    //refreshUsersInRoom();
    refreshMessages();

        //for (int i = 0; i < s_tok.size(); i++) roomsList->addItem(s_tok.at(i));
    //}
    if(!strcmp(room_name, "")){
        usersList->clear();
        allMessages->clear();
    }
    /*
    char message[50];
    sprintf(message,"Timer Refresh New message %d",messageCount);
    allMessages->append(message);
    */

}

void Dialog::refreshRoom(){
    char * resp = (char*) malloc (MAX_RESPONSE * sizeof(char));
    sendCommand(host, port, "GET-ALL-ROOMS", username, pwd, "", resp);
    /*char * s_tok;
    s_tok = strtok (resp, "\r\n");
    roomsList->clear();
    roomsList->addItem(s_tok);
    while (s_tok != NULL){
        printf("%s\n", s_tok);
        s_tok = strtok (NULL, "\r\n");
        roomsList->addItem(s_tok);
    }*/
    QString Qresp = QString::fromStdString(resp);
    if(Qresp.contains("ERROR") ){
        return;
    }
    QStringList rooms_list = Qresp.split("\r\n", QString::SkipEmptyParts);
    roomsList->clear();
    for (int i = 0; i < rooms_list.size(); i++){
        roomsList->addItem(rooms_list.at(i));
    }
}

void Dialog::refreshUsersInRoom(){
    char * resp = (char*) malloc (MAX_RESPONSE * sizeof(char));
    if(limitUsers){
        sendCommand(host, port, "GET-USERS-IN-ROOM", username, pwd, room_name, resp);
    } else
    sendCommand(host, port, "GET-ALL-USERS", username, pwd, "", resp);
    /*char * s_tok;
    s_tok = strtok (resp, "\r\n");
    usersList->clear();
    usersList->addItem(s_tok);
    while (s_tok != NULL){
        printf("%s\n", s_tok);
        s_tok = strtok (NULL, "\r\n");
        usersList->addItem(s_tok);
    }*/
    QString Qresp = QString::fromStdString(resp);
    if(Qresp.contains("ERROR")) return;
    QStringList usersinroom_list = Qresp.split("\r\n", QString::SkipEmptyParts);
    usersList->clear();
    for (int i = 0; i < usersinroom_list.size(); i++){
        usersList->addItem(usersinroom_list.at(i));
    }
}

void Dialog::refreshMessages(){
    char * resp = (char*) malloc (MAX_RESPONSE * sizeof(char));
    sendCommand(host, port, "GET-MESSAGES", username, pwd, strdup((to_string(0) + " " + room_name).c_str()), resp);
    /*char * s_tok;
    s_tok = strtok (resp, "\r\n");
    allMessages->clear();
    allMessages->append(s_tok);
    while (s_tok != NULL){
        printf("%s\n", s_tok);
        s_tok = strtok (NULL, "\r\n");
        allMessages->append(s_tok);
    }*/
    QString Qresp = QString::fromStdString(resp);
    QStringList msgs_list = Qresp.split("\r\n", QString::SkipEmptyParts);
    allMessages->clear();
    for (int i = 0; i < msgs_list.size(); i++){
        allMessages->append(msgs_list.at(i));
    }
}

Dialog::Dialog()
{
    createMenu();

    QVBoxLayout *mainLayout = new QVBoxLayout;

    // Rooms List
    QVBoxLayout * roomsLayout = new QVBoxLayout();
    QLabel * roomsLabel = new QLabel("Rooms");
    roomsList = new QListWidget();
    roomsLayout->addWidget(roomsLabel);
    roomsLayout->addWidget(roomsList);

    username = "admin";
    pwd = "admin";
    //char * str1 = ;
    //strcpy(username, "admin");
    //strcpy(pwd, "admin");
    char * resp = (char *) malloc(sizeof(char)*MAX_RESPONSE);
    sendCommand(host, port, "ADD-USER", "admin", "admin", "", resp);
    //refreshRoom.

    // Users List
    QVBoxLayout * usersLayout = new QVBoxLayout();
    QLabel * usersLabel = new QLabel("Users");
    usersList = new QListWidget();
    usersLayout->addWidget(usersLabel);
    usersLayout->addWidget(usersList);

    // Layout for rooms and users
    QHBoxLayout *layoutRoomsUsers = new QHBoxLayout;
    layoutRoomsUsers->addLayout(roomsLayout);
    layoutRoomsUsers->addLayout(usersLayout);

    // Textbox for all messages
    QVBoxLayout * allMessagesLayout = new QVBoxLayout();
    QLabel * allMessagesLabel = new QLabel("Messages");
    allMessages = new QTextEdit;
    allMessagesLayout->addWidget(allMessagesLabel);
    allMessagesLayout->addWidget(allMessages);

    // Textbox for input message
    QVBoxLayout * inputMessagesLayout = new QVBoxLayout();
    QLabel * inputMessagesLabel = new QLabel("Type your message:");
    inputMessage = new QTextEdit;
    inputMessagesLayout->addWidget(inputMessagesLabel);
    inputMessagesLayout->addWidget(inputMessage);

    // Send and new account buttons
    QHBoxLayout *layoutButtons = new QHBoxLayout;
    QPushButton * sendButton = new QPushButton("Send");
    QPushButton * newUserButton = new QPushButton("New Account");
    QPushButton * loginButton = new QPushButton("Login");
    QPushButton * createRoomButton = new QPushButton("Create Room");
    QPushButton * enterRoomButton = new QPushButton("Enter Room");
    QPushButton * leaveRoomButton = new QPushButton("Leave Room");
    layoutButtons->addWidget(sendButton);
    layoutButtons->addWidget(newUserButton);
    layoutButtons->addWidget(loginButton);
    layoutButtons->addWidget(createRoomButton);
    layoutButtons->addWidget(enterRoomButton);
    layoutButtons->addWidget(leaveRoomButton);

    // Setup actions for buttons
    connect(sendButton, SIGNAL (released()), this, SLOT (sendAction()));
    connect(newUserButton, SIGNAL (released()), this, SLOT (newUserAction()));
    connect(loginButton, SIGNAL (released()), this, SLOT (loginAction()));
    connect(createRoomButton, SIGNAL (released()), this, SLOT (createRoomAction()));
    connect(enterRoomButton, SIGNAL (released()), this, SLOT (enterRoomAction()));
    connect(leaveRoomButton, SIGNAL (released()), this, SLOT (leaveRoomAction()));

    // Add all widgets to window
    mainLayout->addLayout(layoutRoomsUsers);
    mainLayout->addLayout(allMessagesLayout);
    mainLayout->addLayout(inputMessagesLayout);
    mainLayout->addLayout(layoutButtons);



/*
    // Populate rooms
    for (int i = 0; i < 5; i++) {
        char s[50];
        sprintf(s,"Room %d", i);
        roomsList->addItem(s);
    }

    // Populate users
    for (int i = 0; i < 20; i++) {
        char s[50];
        sprintf(s,"User %d", i);
        usersList->addItem(s);
    }

    for (int i = 0; i < 20; i++) {
        char s[50];
        sprintf(s,"Message %d", i);
        allMessages->append(s);
    }
*/

    // Add layout to main window

    setLayout(mainLayout);
    setWindowTitle(tr("CS240 IRC Client"));

    //timer->setInterval(5000);
    messageCount = 0;
    timer = new QTimer(this);
    connect(timer, SIGNAL (timeout()), this, SLOT (timerAction()));
    timer->start(5000);
}


void Dialog::createMenu()

{
    menuBar = new QMenuBar;
    fileMenu = new QMenu(tr("&File"), this);
    exitAction = fileMenu->addAction(tr("E&xit"));
    menuBar->addMenu(fileMenu);

    connect(exitAction, SIGNAL(triggered()), this, SLOT(accept()));
}

