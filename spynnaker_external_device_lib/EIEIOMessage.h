#ifndef _EIEIOMESSAGE_H_
#define _EIEIOMESSAGE_H_

#include <vector>

// eieio type enum
enum EIEIOType {KEY_16_BIT, KEY_PAYLOAD_16_BIT, KEY_32_BIT,
                KEY_PAYLOAD_32_BIT};
enum EIEIOPrefix {LOWER_HALF_WORD, UPPER_HALF_WORD};

// data element
class EIEIOElement {
public:
    int get_key();
    bool has_payload();
    int get_payload();
    int convert_to_bytes(unsigned char * data, int offset);
    EIEIOElement(int key);
    EIEIOElement(int key, int payload);

private:
    int _key;
    int _payload;
    bool _has_payload;
};

// eieio header
class EIEIOHeader {
public:
    int get_p();
    int get_f();
    int get_d();
    int get_t();
    int get_type();
    int get_tag();
    int get_count();
    void increment_count();
    int get_prefix();
    int get_payload_prefix();
    int get_timestamp();
    int get_size();
    static int get_max_size();
    int get_payload_bytes();
    int get_key_bytes();
    int convert_to_bytes(unsigned char *, int offset);
    EIEIOHeader(int p, int f, int d, int t, EIEIOType type, int tag, int count,
                int prefix, int payload_prefix);
    EIEIOHeader(int p, int f, int d, int t, EIEIOType type, int tag, int prefix,
                int payload_prefix);
    static EIEIOHeader* create_from_byte_array(unsigned char *data, int offset);

private:
    int _p;
    int _f;
    int _d;
    int _t;
    int _type;
    int _tag;
    int _count;
    int _prefix;
    int _payload_prefix;
};

// eieio message
class EIEIOMessage {
public:
    EIEIOMessage(unsigned char *data, int offset);
    EIEIOMessage(EIEIOHeader* header);
    bool has_timestamps();
    bool is_next_element();
    EIEIOElement* get_next_element();
    void add_key(int key);
    void add_key_and_payload(int key, int payload);
    int get_size();
    static int get_max_size();
    int get_data(unsigned char* data);

private:
    EIEIOHeader* _header;
    std::vector<EIEIOElement*> _data;
    int _position_read;

    void read_in_16_key_payload_message(
        EIEIOMessage* message, int offset, unsigned char * data);
    void read_in_16_key_message(
        EIEIOMessage* message, int offset, unsigned char * data);
    void read_in_32_key_message(
        EIEIOMessage* message, int offset, unsigned char * data);
    void read_in_32_key_payload_message(
        EIEIOMessage* message, int offset, unsigned char * data);
    int read_element(int offset, unsigned char * data,
                     int number_bytes_to_read);
};

#endif /* _EIEIOMESSAGE_H_ */
