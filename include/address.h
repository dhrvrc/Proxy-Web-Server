
#include <stddef.h> 
#include <stdint.h>

typedef struct addrinfo{
    int ai_flags;
    int ai_family;
    int ai_socktype;  
    int ai_protocol;
    size_t ai_addrlen;
    struct sockaddr *ai_addr;
    char *ai_canonname;
    struct addrinfo *ai_next;
    
};

typedef struct sockaddr{
    unsigned short sa_family;
    char sa_data[14];
};

typedef struct sockaddr_in{
    short sin_family;
    unsigned short sin_port;
    struct in_addr sin_addr;
    char sin_zero[8];
};

typedef struct in_addr
{
    uint32_t s_addr;
};

typedef struct sockaddr_in6{
    short sin6_family;
    unsigned short sin6_port;
    uint32_t sin6_flowinfo;
    struct in6_addr sin6_addr;
    uint32_t sin6_scope_id;
};

typedef struct in6_addr{
    unsigned char s6_addr[16];
};

typedef struct sockaddr_storage{
    unsigned short ss_family;
    char __ss_pad1[_SS_PAD1SIZE];
    int64_t __ss_align;
    char __ss_pad2[_SS_PAD2SIZE];
};




//what do i include for standard c library functions?
