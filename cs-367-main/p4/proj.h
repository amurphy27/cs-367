
#include <stdint.h>

//*****************************************************************************
//************ START OF DO-NOT-MODIFY SECTION *********************************

/*
 * Do not modify code in this section. If you need to add code to this file,
 * add it above or below this section.
 */

#ifdef DEBUG_FLAG
#define DEBUG_wARG(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__)
#define DEBUG_MSG(msg) fprintf(stderr, "%s", msg);
#else
#define DEBUG_wARG(fmt, ...)
#define DEBUG_MSG(msg)
#endif

#define PRINT_wARG(fmt, ...) printf(fmt, __VA_ARGS__); fflush(stdout)
#define PRINT_MSG(msg) printf("%s", msg); fflush(stdout)

int read_stdin(char *buf, int buf_len, int *more);
int parse_mt_bit(uint8_t *header_byte1);
int parse_control_header(uint16_t *header, uint8_t *mt, uint8_t *code, uint8_t *unc, uint8_t *ulen);
int create_control_header(uint16_t *_header, uint8_t mt, uint8_t code, uint8_t unc, uint8_t ulen);
int parse_chat_header(uint32_t *header, uint8_t *mt, uint8_t *pub, uint8_t *prv, uint8_t *frg, uint8_t *lst, uint8_t *ulen, uint16_t *length);
int create_chat_header(uint32_t *header, uint8_t mt, uint8_t pub, uint8_t prv, uint8_t frg, uint8_t lst, uint8_t ulen, uint16_t length);
void print_u16_as_bits(uint16_t x);

// ********************* END OF DO-NOT-MODIFY SECTION *************************

#define PRINT_PUBLIC_MSG(uname, msg) printf("[%s] %s\n", uname, msg); fflush(stdout)
#define PRINT_PRIVATE_MSG(from, to, msg) printf("[%s->%s] %s\n", from, to, msg); fflush(stdout)
#define PRINT_USER_LEFT(uname) printf("%% User %s has left\n", uname); fflush(stdout)
#define PRINT_USER_JOINED(uname) printf("%% User %s has joined\n", uname); fflush(stdout)
#define PRINT_INVALID_RECIPIENT(uname) printf("%% Invalid recipient %s\n", uname); fflush(stdout)