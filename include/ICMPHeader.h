#pragma once

#include <cstdint> // uint64_t
#include <netinet/ip_icmp.h> // icmphdr

// Тип представляющий ICMP пакет.
struct Packet {
    struct icmphdr hdr;

    // Здесь передается время отправки пакета.
    uint64_t data;
};
