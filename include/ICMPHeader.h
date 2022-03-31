#include <cstdint>
#include <netinet/ip_icmp.h>

struct Packet {
    struct icmphdr hdr;
    uint64_t data;
};
