#include <stdint.h>
#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
#else
  #include <arpa/inet.h>
#endif
#include <unistd.h>

#include "varnum.h"
#include "globals.h"
#include "tools.h"

int32_t readVarInt (int client_fd) {
  int32_t value = 0;
  int position = 0;
  uint8_t byte;

  while (true) {
    byte = readByte(client_fd);
    if (recv_count != 1) return VARNUM_ERROR;

    value |= (byte & SEGMENT_BITS) << position;

    if ((byte & CONTINUE_BIT) == 0) break;

    position += 7;
    if (position >= 32) return VARNUM_ERROR;
  }

  return value;
}

int sizeVarInt (uint32_t value) {
  int size = 1;
  while ((value & ~SEGMENT_BITS) != 0) {
    value >>= 7;
    size ++;
  }
  return size;
}

void writeVarInt (int client_fd, uint32_t value) {
  // Encode the whole VarInt up front, then write it out in one go
  uint8_t buf[5];
  int len = 0;
  while (true) {
    if ((value & ~SEGMENT_BITS) == 0) {
      buf[len ++] = value;
      break;
    }
    buf[len ++] = (value & SEGMENT_BITS) | CONTINUE_BIT;
    value >>= 7;
  }
  send_all(client_fd, buf, len);
}
