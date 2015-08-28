#include <EIEIOMessage.h>

// EIEIO elements

// build a basic eieio element
EIEIOElement::EIEIOElement(int key) {
    this->_key = key;
    this->_has_payload = false;
}

// build a eieio element with payload
EIEIOElement::EIEIOElement(int key, int payload) {
    this->_key = key;
    this->_payload = payload;
    this->_has_payload = true;
}

// getters
EIEIOElement:get_key() {
    return this->_key;
}

EIEIOElement:get_payload() {
    return this->_payload;
}

EIEIOElement:has_payload() {
    return this->has_payload;
}

/////////////////// eieio header bits

// constructor for recieving
EIEIOHeader:EIEIOHeader(int p, int f, int d, int t, EIEIOType type, int tag,
                        int count) {
    this->_p = p;
    this->_f = f;
    this->_d = d;
    this->_t = t;
    this->_type = type;
    this->_tag = tag;
    this->_count = count;
}

// constructor for sending
EIEIOHeader:EIEIOHeader(int p, int f, int d, int t, EIEIOType type, int tag){
    this->_p = p;
    this->_f = f;
    this->_d = d;
    this->_t = t;
    this->_type = type;
    this->_tag = tag;
    this->count = 0;
}

//creater from byte array
EIEIOHeader:create_from_byte_array(unsigned char *data, int offset){
    int count = data[offset]
    int header_data = data[offset + 1]

    // Read the flags in the header
    int prefix_flag = (header_data >> 7) & 1;
    int format_flag = (header_data >> 6) & 1;
    int payload_prefix_flag = (header_data >> 5) & 1;
    int payload_is_timestamp = (header_data >> 4) & 1;
    int message_type = (header_data >> 2) & 3;
    int tag = header_data & 3;

    // Check for command header
    if (prefix_flag == 0 and format_flag == 1) {
        return NULL;
    }

    // Convert the flags into types
    EIEIOType eieio_type = EIEIOType(message_type);
    pEIEIOPrefix refix_type = EIEIOPrefix(format_flag);

    int prefix = NULL;
    int payload_prefix = NULL;
    if (payload_prefix_flag == 1) {
        if (eieio_type == EIEIOType.KEY_16_BIT or
                eieio_type == EIEIOType.KEY_PAYLOAD_16_BIT) {
            if (prefix_flag == 1) {
                prefix = (data[offset + 2] >> 8) & 0xFFFF;
                payload_prefix = data[offset + 2] & 0xFFFF;
            }
            else {
                payload_prefix = data[offset + 2]
            }
        }
        else if (eieio_type == EIEIOType.KEY_32_BIT or
                eieio_type == EIEIOType.KEY_PAYLOAD_32_BIT) {
            if (prefix_flag == 1){
                prefix = (data[offset + 2] >> 8) & 0xFFFF;
                payload_prefix = (((data[offset + 2] & 0xFFFF) << 8) +
                                      (data[offset + 3] >> 8) & 0xFFFF);
            }
            else {
                payload_prefix = data[offset + 2]
            }
        }
    }
    else {
        if (prefix_flag == 1) {
            prefix =  prefix = (data[offset + 2] >> 8) & 0xFFFF;
        }
    }

    EIEIOHeader header = EIEIOHeader(p, f, d, t, type, tag, count, prefix,
                                     payload_prefix)
    return header
}

// getters
EIEIOHeader:get_p(){
    return this->_p;
}

EIEIOHeader:get_f(){
    return this->_f;
}

EIEIOHeader:get_d(){
    return this->_d;
}

EIEIOHeader:get_t(){
    return this->_t;
}

EIEIOHeader:get_type(){
    return this->_type;
}

EIEIOHeader:get_tag(){
    return this->_tag;
}

EIEIOHeader:get_count(){
    return this->_count;
}

EIEIOHeader:increment_count(){
    this->_count ++;
}

EIEIOHeader:get_key_bytes(){
    if (this->message_type == EIEIOType.KEY_16_BIT ||
            this->message_type == EIEIOType.KEY_PAYLOAD_16_BIT){
        return 2;
    }
    else {
        return 4;
    }
}

EIEIOHeader:get_payload_bytes(){
    if (this->message_type == EIEIOType.KEY_PAYLOAD_16_BIT){
        return 2;
    }
    else if (this->message_type == EIEIOType.KEY_PAYLOAD_32_BIT){
        return 4;
    }
    else {
        return 0;
    }
}

// gets the size in bytes for the eieio header
EIEIOHeader:get_size(){
    int size = 2;
    if (this->_p == 1) {
        size += 2;
    }
    if (this->_d == 1) {
        if (this->message_type == EIEIOType.KEY_PAYLOAD_16_BIT){
            size += 2;
        }
        else { // works for 32 bit payloads and timestamps.
            size += 4;
        }
    }
    return size;
}

//////////////////////////////////////////////////////////////////////////
EIEIOMessage:EIEIOMessage(unsigned char *data, int offset, int length){
    EIEIOHeader header = EIEIOHeader.create_from_byte_array(data, offset)
    header_offset_end =
    for (int element_id; element_id < header.get_count(); element_id++){
        //TODO THIS NEEDS DOING< AND NEEDS TO ACCOUTN FOR WORD ALIGMEMENT
    }
}

EIEIOMessage:EIEIOMessage(EIEIOHeader header){
    this->_header = header;
}

//! \brief converts header t into a boolean for easy use from message
EIEIOMessage:has_timestamps(){
    if (this->header->get_t() == 1) {
        return true;
    }
    else {
        return false;
    }
}

//! \brief helper for seeing if theres still elements in the list to read
EIEIOMessage:is_next_element(){
    if (this->_position_read == this->_header->count()){
        return false;
    }
    else {
        return true;
    }
}

//! \biref gets the next element in the data
EIEIOMessage:get_next_element() {
    if this->is_next_element() {
        _position_read ++;
        return this->_data[this->_position_read - 1];
    }
    else {
        return NULL
    }
}

//! \biref adds a key to the data object
EIEIOMessage:add_key(int key){
    EIEIOElement element = EIEIOElement(key);
    this->_data.push_back(element);
    this->_header->increment_count();
}

//! \biref adds a key and payload to the data object
EIEIOMessage:add_key(int key, int payload){
    EIEIOElement element = EIEIOElement(key, payload);
    this->_data.push_back(element);
    this->_header->increment_count();
}

//! \biref gets the size of the eieio message in bytes
EIEIOMessage:get_size(){
    int size = 0;
    size += this->_header->size();
    size += ((this->_header->get_key_bytes() +
              this->_header->get_payload_bytes())
             * this->_header->get_count())
    return size;
}