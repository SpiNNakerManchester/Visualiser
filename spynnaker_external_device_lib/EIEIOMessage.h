#ifndef _EIEIOMESSAGE_H_
#define _EIEIOMESSAGE_H_

class EIEIOElement {
public:
    int get_key();
    bool has_payload();
    int get_payload();

protected:
    EIEIOElement(int key);
    EIEIOElement(int key, int payload);

private:
    int _key;
    int _payload;
    bool _has_payload;
};

class EIEIOMessage {
public:
    EIEIOMessage(unsigned char *data, int offset, int length);
    bool has_timestamps();
    bool is_next_element();
    EIEIOElement *get_next_element();
};

#endif /* _EIEIOMESSAGE_H_ */
