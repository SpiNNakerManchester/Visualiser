#ifndef EIEIOMESSAGE_H_

struct eieio_header{
 	int p;
    int f;
    int d;
    int t;
    int type;
    int tag;
    int count;
};


struct eieio_message{
    eieio_header header;
    char* data;
};

struct eieio_command_header{
	int p;
    int f;
    int command;
};

struct eieio_command_message{
    eieio_command_header header;
    char* data;
};

#endif /* EIEIOMESSAGE_H_ */
#define EIEIOMESSAGE_H_