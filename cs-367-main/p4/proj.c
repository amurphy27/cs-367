
#include <stdio.h>
#include <string.h>

#include "proj.h"

int read_stdin(char *buf, int buf_len, int *more) {
    int tot_chars = 0;
    if (fgets(buf, buf_len, stdin))
    {
        tot_chars = strlen(buf);
    }
    *more = 1;
    if (strchr(buf, '\n'))
    {
        *more = 0;
    }
    return tot_chars;
}

/**
 * Helper function that prints binary representation of the given int.
 * This can be useful for debugging when implementing header parsing/creation functions.
 *
 * You may want to write your own print_u32_as_bits(uint32_t x) function.
 *
 * @param x
 */
void print_u16_as_bits(uint16_t x) {
    printf("%u: ", x);
    for (int i = 15; i >= 0; i--) {
        printf("%u ", (x & (0x0001 << i)) >> i);
    }
    printf("\n");
}

/**
 * Parse the MT bit in the given first byte of the header.
 *
 * @param header_byte1
 * @return -1 on failure (e.g., header_byte1 is NULL). Otherwise MT bit value (0 or 1)
 */
int parse_mt_bit(uint8_t *header_byte1) {
    if (header_byte1 == NULL)
    {
        return -1;
    }
    return *header_byte1 >> 7;
}


/**
 * Parse 2 byte control header.
 *
 * This function simply parses (or unpacks) all the fields fields in a control header. It shouldn't do any validation for the field values. For example, as per
 * specifications ulen should not be greater than 11. But this function shouldn't do that validation. Validation should be done elsewhere.
 *
 * @param header: pointer to a 2-byte header buffer
 * @param mt: Result parameter that'll be filled with Message type field value in the header
 * @param code: Result parameter that'll be filled with CODE field value in the header
 * @param unc: Result parameter that'll be filled with UNC field value in the header
 * @param ulen: Result parameter that'll be filled with ULEN field value in the header
 *
 * @return 1 on success, -1 on failure (e.g., header is NULL).
 */
int parse_control_header(uint16_t *header, uint8_t *mt, uint8_t *code, uint8_t *unc, uint8_t *ulen) {
    if (header == NULL)
    {
        return -1;
    }
    int ulenMask = 0xF;
    int uncMask = 0xF0;
    int codeMask = 0x7800;
    int mtMask = 0x8000;
    *ulen = ulenMask & *header;
    *unc = (uncMask & *header) >> 4;
    *code = (codeMask & *header) >> 11;
    *mt = (mtMask & *header) >> 15;
    return 1;
}

/**
 * Create control message header
 *
 * This function simply packs all the given fields to create a control header. It shouldn't do any validation for the field values. For example, as per
 * specifications ulen should not be greater than 11. But this function shouldn't do that validation. Validation should be done elsewhere.
 *
 * @param header: Pointer to an allocated 2-byte header buffer. This will be filled with the header bytes.
 * @param mt: Message type field value
 * @param code: CODE field value
 * @param unc: UNC field value
 * @param ulen: Username length field value
 *
 * @return 1 on success, -1 on failure (e.g., header is NULL).
 */
int create_control_header(uint16_t *_header, uint8_t mt, uint8_t code, uint8_t unc, uint8_t ulen) {
    if (_header == NULL)
    {
        return -1;
    }
    //*_header = ((mt | code) << 8) | (unc | ulen);
    *_header = (mt << 15) | (code << 11) | (unc << 4) | ulen;
    return 1;
}


/**
 * Parse chat message header
 *
 * This function simply parses (or unpacks) all the fields fields in a chat header. It shouldn't do any validation for the field values. For example, as per
 * specifications ulen should not be greater than 11. But this function shouldn't do that validation. Validation should be done elsewhere.
 *
 * @return 1 on success, 0 on failure.
 */
int parse_chat_header(uint32_t *header, uint8_t *mt, uint8_t *pub, uint8_t *prv, uint8_t *frg, uint8_t *lst, uint8_t *ulen, uint16_t *length) {
    if (header == NULL)
    {
        return 0;
    }
    int mtMask = 0x80000000;
    int pubMask = 0x40000000;
    int prvMask = 0x20000000;
    int frgMask = 0x10000000;
    int lstMask = 0x8000000;
    int ulenMask = 0xF000;
    int lengthMask = 0xFFF;
    *length = lengthMask & *header;
    *ulen = (ulenMask & *header) >> 12;
    *lst = (lstMask & *header) >> 27;
    *frg = (frgMask & *header) >> 28;
    *prv = (prvMask & *header) >> 29;
    *pub = (pubMask & *header) >> 30;
    *mt = (mtMask & *header) >> 31;
    return 1;
}


/**
 * Create chat message header.
 *
 * This function simply packs all the given fields to create a chat header. It shouldn't do any validation for the field values. For example, as per
 * specifications ulen should not be greater than 11. But this function shouldn't do that validation. Validation should be done elsewhere.
 *
 * @return 1 on success, -1 on failure (e.g., header is NULL).
 */
int create_chat_header(uint32_t *header, uint8_t mt, uint8_t pub, uint8_t prv, uint8_t frg, uint8_t lst, uint8_t ulen, uint16_t length) {
    if (header == NULL)
    {
        return -1;
    }
    //*header = ((((mt | pub | prv | frg | lst) << 16) | ulen) << 8) | length;
    *header = (mt << 31) | (pub << 30) | (prv << 29) | (frg << 28) | (lst << 27) | (ulen << 12) | length;
    return 1;
}

//this is the main that I used to print my tests to make sure all the functions in here work properly
/*int main(int argc, char** argv)
{
    //test for parse_mt_bit
    uint8_t *testnull = NULL;
    uint8_t test;
    printf("parse_mt_bit null is: %d\n", parse_mt_bit(testnull));
    test = 0x80;
    printf("parse_mt_bit 0x80 is: %d\n", parse_mt_bit(&test));
    test = 0x01;
    printf("parse_mt_bit 0x01 is: %d\n\n\n", parse_mt_bit(&test));

    //test for parse_control_header
    uint16_t *test16null = NULL;
    uint8_t mt, code, unc, ulen;
    printf("parse_control_header null is: %d\n", parse_control_header(test16null, &mt, &code, &unc, &ulen));
    uint16_t test2 = 0x8000;
    printf("parse_control_header 0x8000 is: %d\n", parse_control_header(&test2, &mt, &code, &unc, &ulen));
    printf("mt: %d   code: %d   unc: %d   ulen: %d\n", mt, code, unc, ulen);
    test2 = 0x7800;
    printf("parse_control_header 0x7800 is: %d\n", parse_control_header(&test2, &mt, &code, &unc, &ulen));
    printf("mt: %d   code: %d   unc: %d   ulen: %d\n", mt, code, unc, ulen);
    test2 = 0xFFFF;
    printf("parse_control_header 0xFFFF is: %d\n", parse_control_header(&test2, &mt, &code, &unc, &ulen));
    printf("mt: %d   code: %d   unc: %d   ulen: %d\n\n\n", mt, code, unc, ulen);

    //test for create_control_header
    printf("create_control_header null is: %d\n", create_control_header(test16null, mt, code, unc, ulen));
    uint16_t test3;
    mt = 0;
    code = 0;
    unc = 0;
    ulen = 0;
    create_control_header(&test3, mt, code, unc, ulen);
    printf("create_control_header 0s is: %d\n", test3);
    mt = 0x01;
    code = 0x0F;
    unc = 0x0F;
    ulen = 0x0F;
    create_control_header(&test3, mt, code, unc, ulen);
    printf("create_control_header 1s is: %d\n", test3);
    code = 0;
    ulen = 0;
    create_control_header(&test3, mt, code, unc, ulen);
    printf("create_control_header mix is: %d\n", test3);
    mt = 0;
    code = 0x0F;
    unc = 0;
    ulen = 0x0F;
    create_control_header(&test3, mt, code, unc, ulen);
    printf("create_control_header mix is: %d\n\n\n", test3);

    //test for parse_chat_header
    uint32_t *test32null = NULL;
    uint8_t pub, prv, frg, lst;
    uint16_t length;
    printf("parse_chat_header null is: %d\n", parse_chat_header(test32null, &mt, &pub, &prv, &frg, &lst, &ulen, &length));
    mt = 0;
    ulen = 0;
    uint32_t test4 = 0;
    printf("parse_chat_header 0s is: %d\n", parse_chat_header(&test4, &mt, &pub, &prv, &frg, &lst, &ulen, &length));
    printf("mt: %d   pub: %d   prv: %d   frg: %d   lst: %d   ulen: %d   length: %d\n", mt, pub, prv, frg, lst, ulen, length);
    test4 = 0xFFFFFFFF;
    printf("parse_chat_header 1s is: %d\n", parse_chat_header(&test4, &mt, &pub, &prv, &frg, &lst, &ulen, &length));
    printf("mt: %d   pub: %d   prv: %d   frg: %d   lst: %d   ulen: %d   length: %d\n", mt, pub, prv, frg, lst, ulen, length);
    test4 = 0x80000000;
    printf("parse_chat_header mt is: %d\n", parse_chat_header(&test4, &mt, &pub, &prv, &frg, &lst, &ulen, &length));
    printf("mt: %d   pub: %d   prv: %d   frg: %d   lst: %d   ulen: %d   length: %d\n", mt, pub, prv, frg, lst, ulen, length);
    test4 = 0x40000000;
    printf("parse_chat_header pub is: %d\n", parse_chat_header(&test4, &mt, &pub, &prv, &frg, &lst, &ulen, &length));
    printf("mt: %d   pub: %d   prv: %d   frg: %d   lst: %d   ulen: %d   length: %d\n", mt, pub, prv, frg, lst, ulen, length);
    test4 = 0x20000000;
    printf("parse_chat_header prv is: %d\n", parse_chat_header(&test4, &mt, &pub, &prv, &frg, &lst, &ulen, &length));
    printf("mt: %d   pub: %d   prv: %d   frg: %d   lst: %d   ulen: %d   length: %d\n", mt, pub, prv, frg, lst, ulen, length);
    test4 = 0x10000000;
    printf("parse_chat_header frg is: %d\n", parse_chat_header(&test4, &mt, &pub, &prv, &frg, &lst, &ulen, &length));
    printf("mt: %d   pub: %d   prv: %d   frg: %d   lst: %d   ulen: %d   length: %d\n", mt, pub, prv, frg, lst, ulen, length);
    test4 = 0x8000000;
    printf("parse_chat_header lst is: %d\n", parse_chat_header(&test4, &mt, &pub, &prv, &frg, &lst, &ulen, &length));
    printf("mt: %d   pub: %d   prv: %d   frg: %d   lst: %d   ulen: %d   length: %d\n", mt, pub, prv, frg, lst, ulen, length);
    test4 = 0xF000;
    printf("parse_chat_header ulen is: %d\n", parse_chat_header(&test4, &mt, &pub, &prv, &frg, &lst, &ulen, &length));
    printf("mt: %d   pub: %d   prv: %d   frg: %d   lst: %d   ulen: %d   length: %d\n", mt, pub, prv, frg, lst, ulen, length);
    test4 = 0xFFF;
    printf("parse_chat_header length is: %d\n", parse_chat_header(&test4, &mt, &pub, &prv, &frg, &lst, &ulen, &length));
    printf("mt: %d   pub: %d   prv: %d   frg: %d   lst: %d   ulen: %d   length: %d\n\n\n", mt, pub, prv, frg, lst, ulen, length);

    //test for create_chat_header
    printf("create_chat_header null is: %d\n", create_chat_header(test32null, mt, pub, prv, frg, lst, ulen, length));
    uint32_t test5;
    mt = 0;
    pub = 0;
    prv = 0;
    frg = 0;
    lst = 0;
    ulen = 0;
    length = 0;
    create_chat_header(&test5, mt, pub, prv, frg, lst, ulen, length);
    printf("create_chat_header 0s is: %d\n", test5);
    mt = 0x01;
    pub = 0x01;
    prv = 0x01;
    frg = 0x01;
    lst = 0x01;
    ulen = 0xF;
    length = 0xFFF;
    create_chat_header(&test5, mt, pub, prv, frg, lst, ulen, length);
    printf("create_chat_header 1s is: %d\n", test5);
    pub = 0;
    prv = 0;
    frg = 0;
    lst = 0;
    ulen = 0;
    length = 0;
    create_chat_header(&test5, mt, pub, prv, frg, lst, ulen, length);
    printf("create_chat_header mt is: %d\n", test5);
    mt = 0;
    pub = 0x01;
    create_chat_header(&test5, mt, pub, prv, frg, lst, ulen, length);
    printf("create_chat_header pub is: %d\n", test5);
    pub = 0;
    prv = 0x01;
    create_chat_header(&test5, mt, pub, prv, frg, lst, ulen, length);
    printf("create_chat_header prv is: %d\n", test5);
    prv = 0;
    frg = 0x01;
    create_chat_header(&test5, mt, pub, prv, frg, lst, ulen, length);
    printf("create_chat_header frg is: %d\n", test5);
    frg = 0;
    lst = 0x01;
    create_chat_header(&test5, mt, pub, prv, frg, lst, ulen, length);
    printf("create_chat_header lst is: %d\n", test5);
    lst = 0;
    ulen = 0xF;
    create_chat_header(&test5, mt, pub, prv, frg, lst, ulen, length);
    printf("create_chat_header ulen is: %d\n", test5);
    ulen = 0;
    length = 0xFFF;
    create_chat_header(&test5, mt, pub, prv, frg, lst, ulen, length);
    printf("create_chat_header length is: %d\n", test5);
    return 1;
}*/
//end of test main
