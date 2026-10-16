#include "../interpreter.h"

#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include <string.h>

void Socket_Socket(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs) {
    int domain   = Param[0]->Val->Integer;
    int type     = Param[1]->Val->Integer;
    int protocol = Param[2]->Val->Integer;
    
    ReturnValue->Val->Integer = lwip_socket(domain, type, protocol);
}

void Socket_Close(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs) {
    int fd = Param[0]->Val->Integer;
    ReturnValue->Val->Integer = lwip_close(fd);
}

void Socket_Read(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs) {
    int fd      = Param[0]->Val->Integer;
    char *buf   = (char *)(Param[1]->Val->Pointer);
    int count   = Param[2]->Val->Integer;
    
    ReturnValue->Val->Integer = lwip_read(fd, buf, count);
}

void Socket_Write(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs) {
    int fd            = Param[0]->Val->Integer;
    const char *buf   = (const char *)(Param[1]->Val->Pointer);
    int count         = Param[2]->Val->Integer;
    
    ReturnValue->Val->Integer = lwip_write(fd, buf, count);
}

void Socket_Connect(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs) {
    int fd            = Param[0]->Val->Integer;
    const char *host  = (const char *)(Param[1]->Val->Pointer);
    int port          = Param[2]->Val->Integer;
    
    struct sockaddr_in server_addr;
    struct hostent *server;
    
    server = lwip_gethostbyname(host);
    if (server == NULL) {
        ReturnValue->Val->Integer = -1;
        return;
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    server_addr.sin_port = lwip_htons(port);
    
    ReturnValue->Val->Integer = lwip_connect(fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
}

struct LibraryFunction SocketFunctions[] = {
    { Socket_Socket,  "int socket(int, int, int);" },
    { Socket_Close,   "int close(int);" },
    { Socket_Read,    "int read(int, char *, int);" },
    { Socket_Write,   "int write(int, char *, int);" },
    { Socket_Connect, "int connect_to(int, char *, int);" },
    { NULL,           NULL }
};

const char SocketDefs[] =
        "struct in_addr { unsigned long s_addr; };\n"
        "struct sockaddr_in {\n"
        "    short sin_family;\n"
        "    unsigned short sin_port;\n"
        "    struct in_addr sin_addr;\n"
        "    char sin_zero[8];\n"
        "};\n"
        "struct sockaddr { unsigned short sa_family; char sa_data[14]; };\n";

void SocketSetupFunc(Picoc *pc)
{
    VariableDefinePlatformVar(pc, NULL, "AF_INET", &pc->IntType, (union AnyValue *)&((int){AF_INET}), FALSE);
    VariableDefinePlatformVar(pc, NULL, "AF_INET6", &pc->IntType, (union AnyValue *)&((int){AF_INET6}), FALSE);
    VariableDefinePlatformVar(pc, NULL, "SOCK_STREAM", &pc->IntType, (union AnyValue *)&((int){SOCK_STREAM}), FALSE);
    VariableDefinePlatformVar(pc, NULL, "SOCK_DGRAM", &pc->IntType, (union AnyValue *)&((int){SOCK_DGRAM}), FALSE);
    VariableDefinePlatformVar(pc, NULL, "IPPROTO_TCP", &pc->IntType, (union AnyValue *)&((int){IPPROTO_TCP}), FALSE);
    VariableDefinePlatformVar(pc, NULL, "IPPROTO_UDP", &pc->IntType, (union AnyValue *)&((int){IPPROTO_UDP}), FALSE);
}
