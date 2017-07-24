#include "EIEIOMessage.h"
#include <stddef.h>
#include <stdexcept>
#include <stdlib.h>
#include <stdio.h>

// EIEIO elements

// build a basic eieio element
EIEIOElement::EIEIOElement(int key) {
    this->_key = key;
    this->_payload = 0;
    this->_has_payload = false;
}

// build a eieio element with payload
EIEIOElement::EIEIOElement(int key, int payload) {
    this->_key = key;
    this->_payload = payload;
    this->_has_payload = true;
}

// getters
int EIEIOElement::get_key() {
    return this->_key;
}

int EIEIOElement::get_payload() {
    return this->_payload;
}

bool EIEIOElement::has_payload() {
    return this->_has_payload;
}

//! \brief converts eieieo elment into bytes for transmission via sockets
int EIEIOElement::convert_to_bytes(unsigned char* data, int offset, int format) {
    if (format == KEY_16_BIT || format == KEY_PAYLOAD_16_BIT){
        data[offset] = (unsigned char) (this->_key & 0xFFFF);
        data[offset + 1] = (unsigned char) ((this->_key >> 8) & 0xFFFF);
        offset += 2;
    }
    else if (format == KEY_32_BIT || format == KEY_PAYLOAD_32_BIT){
        data[offset] = (unsigned char) (this->_key & 0xFFFF);
        data[offset + 1] = (unsigned char) ((this->_key >> 8) & 0xFFFF);
        data[offset + 2] = (unsigned char) ((this->_key >> 16) & 0xFFFF);
        data[offset + 3] = (unsigned char) ((this->_key >> 24) & 0xFFFF);
        offset += 4;
    }
    else {
        throw "Invalid format type, please fix and try again";
    }
    if (this->_has_payload){
        if (format == KEY_PAYLOAD_16_BIT){
            data[offset] = (unsigned char) (this->_payload & 0xFFFF);
            data[offset + 1] = (unsigned char) ((this->_payload >> 8) & 0xFFFF);
            offset += 2;
        }
        else if (format == KEY_PAYLOAD_32_BIT){
           data[offset] = (unsigned char) (this->_payload & 0xFFFF);
            data[offset + 1] = (unsigned char) ((this->_payload >> 8) & 0xFFFF);
            data[offset + 2] = (unsigned char) ((this->_payload >> 16) & 0xFFFF);
            data[offset + 3] = (unsigned char) ((this->_payload >> 24) & 0xFFFF);
            offset += 4;
        }
        else{
            throw "Invalid format type, please fix and try again";
        }
    }
    return offset;
}

/////////////////// eieio header bits

// constructor for recieving
EIEIOHeader::EIEIOHeader(int p, int f, int d, int t, EIEIOType type, int tag,
                         int count, int prefix, int payload_prefix) {
    this->_p = p;
    this->_f = f;
    this->_d = d;
    this->_t = t;
    this->_type = type;
    this->_tag = tag;
    this->_count = count;
    this->_prefix = prefix;
    this->_payload_prefix = payload_prefix;
}

// constructor for sending
EIEIOHeader::EIEIOHeader(
        int p, int f, int d, int t, EIEIOType type, int tag, int prefix,
        int payload_prefix){
    this->_p = p;
    this->_f = f;
    this->_d = d;
    this->_t = t;
    this->_type = type;
    this->_tag = tag;
    this->_count = 0;
    this->_prefix = prefix;
    this->_payload_prefix = payload_prefix;
}

//creater from byte array
EIEIOHeader* EIEIOHeader::create_from_byte_array(
        unsigned char *data, int offset){
    int count = data[offset];
    int header_data = data[offset + 1];

    // Read the flags in the header
    int prefix_flag = (header_data >> 7) & 1;
    int format_flag = (header_data >> 6) & 1;
    int payload_prefix_flag = (header_data >> 5) & 1;
    int payload_is_timestamp = (header_data >> 4) & 1;
    int message_type = (header_data >> 2) & 3;
    int tag = header_data & 3;

    // Check for command header
    if (prefix_flag == 0 and format_flag == 1) {
        throw std::invalid_argument("not currently supporting command packets");
    }

    // Convert the flags into types
    EIEIOType eieio_type = EIEIOType(message_type);
    EIEIOPrefix refix_type = EIEIOPrefix(format_flag);

    int prefix = -1;
    int payload_prefix = -1;
    if (payload_prefix_flag == 1) {
        if (eieio_type == KEY_16_BIT or
                eieio_type == KEY_PAYLOAD_16_BIT) {
            if (prefix_flag == 1) {
                prefix = (data[offset + 2] >> 8) & 0xFFFF;
                payload_prefix = data[offset + 2] & 0xFFFF;
            }
            else {
                payload_prefix = data[offset + 2];
            }
        }
        else if (eieio_type == KEY_32_BIT or
                eieio_type == KEY_PAYLOAD_32_BIT) {
            if (prefix_flag == 1){
                prefix = (data[offset + 2] >> 8) & 0xFFFF;
                payload_prefix = EIEIOHeader::read_element(offset + 2, data, 4);
            }
            else {
            	payload_prefix = EIEIOHeader::read_element(offset + 2, data, 4);
            }
        }
    }
    else {
        if (prefix_flag == 1) {
            prefix =  prefix = (data[offset + 2] >> 8) & 0xFFFF;
        }
    }

    EIEIOHeader* header = new EIEIOHeader(
        prefix_flag, format_flag, payload_prefix_flag, payload_is_timestamp,
        eieio_type, tag, count, prefix, payload_prefix);
    return header;
}

//! \brief reads a number of bytes and converts them into a int
int EIEIOHeader::read_element(
        int offset, unsigned char * data, int number_bytes_to_read){
    int final_data = 0;
    for (int position = 0; position < number_bytes_to_read; position++){
        final_data += (data[(offset + position)] << (position * 8));
    }
    return final_data;
}

// getters
int EIEIOHeader::get_p(){
    return this->_p;
}

int EIEIOHeader::get_f(){
    return this->_f;
}

int EIEIOHeader::get_d(){
    return this->_d;
}

int EIEIOHeader::get_t(){
    return this->_t;
}

int EIEIOHeader::get_type(){
    return this->_type;
}

int EIEIOHeader::get_tag(){
    return this->_tag;
}

int EIEIOHeader::get_count(){
    return this->_count;
}

void EIEIOHeader::increment_count(){
    this->_count ++;
}

int EIEIOHeader::get_timestamp(){
    if (this->_t == 1){
        return this->_payload_prefix;
    }
    else{
        throw "This message does not contain a time stamp";
    }
}

int EIEIOHeader::get_key_bytes(){
    if (this->_type == KEY_16_BIT ||
            this->_type == KEY_PAYLOAD_16_BIT){
        return 2;
    }
    else {
        return 4;
    }
}

int EIEIOHeader::get_payload_bytes(){
    if (this->_type == KEY_PAYLOAD_16_BIT){
        return 2;
    }
    else if (this->_type == KEY_PAYLOAD_32_BIT){
        return 4;
    }
    else {
        return 0;
    }
}

//! \brief converts a eieio header into bytes for sending via sockets
int EIEIOHeader::convert_to_bytes(unsigned char * data, int offset){
    data[offset] = (unsigned char) this->_count;
    int header_part = 0;
    header_part += (this->_p << 7);
    header_part += (this->_f << 6);
    header_part += (this->_d << 5);
    header_part += (this->_t << 4);
    header_part += (this->_type << 2);
    header_part += this->_tag;
    data[offset + 1] = (unsigned char) header_part;
    return offset + 2;
}

// gets the size in bytes for the eieio header
int EIEIOHeader::get_size(){
    int size = 2;
    if (this->_p == 1) {
        size += 2;
    }
    if (this->_d == 1) {
        if (this->_type == KEY_PAYLOAD_16_BIT){
            size += 2;
        }
        else { // works for 32 bit payloads and timestamps.
            size += 4;
        }
    }
    return size;
}

// gets the max posible size in bytes for a eieio header
int EIEIOHeader::get_max_size(){
    return 8; //! 1 for count, 1 for header spec, 2 for key_prefix, 4 for payload_prefix
}

//////////////////////////////////////////////////////////////////////////
EIEIOMessage::EIEIOMessage(unsigned char *data, int offset) {
    EIEIOHeader* header = EIEIOHeader::create_from_byte_array(data, offset);
    offset = offset + header->get_size();
    this->_header = header;

    int element_id = 0;
    int count = header->get_count();
    while (element_id < count) {
        if (header->get_type() == KEY_PAYLOAD_16_BIT) {
            read_in_16_key_payload_message(offset, data);
            offset += 4;
        }
        else if ( header->get_type() == KEY_16_BIT) {
            read_in_16_key_message(offset, data);
            offset += 2;
        }
        else if ( header->get_type() == KEY_32_BIT) {
            read_in_32_key_message(offset, data);
            offset += 4;
        }
        else if ( header->get_type() == KEY_PAYLOAD_32_BIT) {
           read_in_32_key_payload_message(offset, data);
           offset += 8;
        }
        element_id+=1;
    }
    this->_position_read = 0;
}

//! \brief reads in a EIEIO element from the data with 16 bit key and
//! 16 bit payload.
void EIEIOMessage::read_in_16_key_payload_message(
        int offset, unsigned char * data){
    int key = read_element(offset, data, 2);
    int payload = read_element(offset + 2, data, 2);
    this->add_key_and_payload(key, payload);
}

//! \brief reads in a EIEIO element from the data with 16 bit key.
void EIEIOMessage::read_in_16_key_message(
        int offset, unsigned char * data){
    int key = read_element(offset, data, 2);
    if (this->_header->get_t() == 1) {
        this->add_key_and_payload(key, this->_header->get_timestamp());
    }
    else{
        this->add_key(key);
    }
}

//! \brief reads in a EIEIO element from the data with 32 bit key.
void EIEIOMessage::read_in_32_key_message(
        int offset, unsigned char * data){
    int key = read_element(offset, data, 4);
    if (this->_header->get_t() == 1){
        this->add_key_and_payload(key, this->_header->get_timestamp());
    }
    else {
        this->add_key(key);
    }
}

//! \brief reads in a EIEIO element from the data with 32 bit key and
//! 32 bit payload.
void EIEIOMessage::read_in_32_key_payload_message(
        int offset, unsigned char * data){
    int key = read_element(offset, data, 4);
    int payload = read_element(offset + 4, data, 4);
    this->add_key_and_payload(key, payload);
}

//! \brief reads a number of bytes and converts them into a int
int EIEIOMessage::read_element(
        int offset, unsigned char * data, int number_bytes_to_read){
    int final_data = 0;
    for (int position = 0; position < number_bytes_to_read; position++){
        final_data |= (data[(offset + position)] << (position * 8));
    }
    return final_data;
}

EIEIOMessage::EIEIOMessage(EIEIOHeader* header){
    this->_header = header;
    this->_position_read = 0;
}

//! \brief converts header t into a boolean for easy use from message
bool EIEIOMessage::has_timestamps(){
    if (this->_header->get_t() == 1) {
        return true;
    }
    else {
        return false;
    }
}

//! \brief adds one to the header
void EIEIOMessage::increment_count(){
    this->_header->increment_count();
}

//! \brief helper for seeing if theres still elements in the list to read
bool EIEIOMessage::is_next_element(){
    if (this->_position_read == this->_header->get_count()){
        return false;
    }
    else {
        return true;
    }
}

//! \biref gets the next element in the data
EIEIOElement* EIEIOMessage::get_next_element() {
    if (this->is_next_element()) {
        _position_read ++;
        return this->_data[this->_position_read - 1];
    }
    else {
        throw std::invalid_argument("No more elements");
    }
}

//! \biref adds a key to the data object
void EIEIOMessage::add_key(int key){
    EIEIOElement* element = new EIEIOElement(key);
    this->_data.push_back(element);
}

//! \biref adds a key and payload to the data object
void EIEIOMessage::add_key_and_payload(int key, int payload){
    EIEIOElement* element = new EIEIOElement(key, payload);
    this->_data.push_back(element);
}

//! \biref gets the size of the eieio message in bytes
int EIEIOMessage::get_size(){
    int size = 0;
    size += this->_header->get_size();
    size += ((this->_header->get_key_bytes() +
              this->_header->get_payload_bytes())
             * this->_header->get_count());
    return size;
}

//! \biref gets the max possible size of a eieio message in bytes
int EIEIOMessage::get_max_size(){
    int size = 0;
    size += EIEIOHeader::get_max_size();
    size += 256 * 8;
    return size;
}

//! \brief returns the ehasder and data as a char * for sending via sockets
int EIEIOMessage::get_data(unsigned char * data){
    int offset = 0;
    offset = this->_header->convert_to_bytes(data, offset);
    for(int position = 0; position < this->_data.size(); position ++) {
        offset = this->_data[position]->convert_to_bytes(
            data, offset, this->_header->get_type());
    }
    return offset;
}
