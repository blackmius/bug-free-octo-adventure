#pragma once

#include <cstdint>
#include <netinet/ip_icmp.h>

// Тип представляющий ICMP пакет.
struct Packet {
    struct icmphdr hdr;

    // Здесь передается время отправки пакета.
    uint64_t data;
};
