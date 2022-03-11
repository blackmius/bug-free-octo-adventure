#include <cstdint>

struct icmp_hdr_t {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint32_t data;   
};
