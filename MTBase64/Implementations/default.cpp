#include "MTBase64.hpp"


struct MTBase64::IndexTableAccessor {
    /*Reverse chunking of four 6 bit bytes to three 8 bit bytes by using reverse
    lookup table and padding checking*/
    static void DecodeBase64(uint8_t *dest, const uint8_t *src, std::size_t src_len,
                             const MTBase64::IndexTable& table,
                             bool padding = true) {

        if (padding && !MTBase64::ValidPaddedEncodedLength(src_len))
            throw MTBase64::MTBase64Exception(
            __FILE__, __FUNCTION__, __LINE__,
            MTBase64::ErrorCodeTable::kNotValidBase64,
            "Not valid base64 encoding length when padding is being used.");

        if (!padding && !MTBase64::ValidUnpaddedEncodedLength(src_len))
            throw MTBase64::MTBase64Exception(
            __FILE__, __FUNCTION__, __LINE__,
            MTBase64::ErrorCodeTable::kNotValidBase64,
            "Not valid base64 encoding length when padding is not being used.");

        uint8_t padding_byte = table.GetPadding();
        for (size_t e_bc = 0, d_bc = 0; e_bc < src_len;) {

            /*Padding is being used as end of data identifier even when
            `padding = false`*/
            uint8_t eb1 = src[e_bc++];
            uint8_t eb2 = src[e_bc++];
            uint8_t eb3 = (e_bc == src_len) ? padding_byte : src[e_bc++];
            uint8_t eb4 = (e_bc == src_len) ? padding_byte : src[e_bc++];

            /*Low level stuff */
            uint8_t to_add = 3 - (eb4 == padding_byte) - (eb3 == padding_byte);
            /*First use of [[fallthrough]]*/
            switch (to_add) {
            case 3:
                dest[d_bc + 2] = (
                (table.ReverseLookup(eb3) << 6) | table.ReverseLookup(eb4)
                );
                [[fallthrough]];
            case 2:
                dest[d_bc + 1] = (
                (table.ReverseLookup(eb2) << 4) | (table.ReverseLookup(eb3) >> 2)
                );
                [[fallthrough]];
            case 1:
                dest[d_bc + 0] = (
                (table.ReverseLookup(eb1) << 2) | (table.ReverseLookup(eb2) >> 4)
                );
                [[fallthrough]];
            default:
                d_bc += to_add;
                break;
            }
        }
    }

    /*Splits the encoding process into the encoding of whole 3-byte chunks and the
    encoding of the last 1-2 byte chunk with padding at the end if used*/
    static void EncodeBase64(uint8_t *dest, const uint8_t *src, std::size_t src_len,
                             const MTBase64::IndexTable& table,
                             bool padding = true) {

        if(src_len == 0)
            throw MTBase64::MTBase64Exception(
            __FILE__, __FUNCTION__, __LINE__,
            MTBase64::ErrorCodeTable::kIllegalFunctionCall,
            "input buffer length is 0.");

        uint8_t padding_byte    = table.GetPadding(),   remainder   = src_len % 3;
        size_t d_bc             = 0,                    e_bc        = 0;


        uint8_t db1, db2, db3;
        /*The loop bellow encodes only whole chunks of 3 bytes*/
        while (d_bc < src_len - remainder) {
            db1 = src[d_bc++];
            db2 = src[d_bc++];
            db3 = src[d_bc++];

            dest[e_bc++] = table.e0.at(db1);
            dest[e_bc++] = table.e1.at(((db1 & 0x03) << 4) | ((db2 >> 4) & 0x0F));
            dest[e_bc++] = table.e1.at(((db2 & 0x0F) << 2) | ((db3 >> 6) & 0x03));
            dest[e_bc++] = table.e2.at(db3);
        }

        switch (remainder) {
        case 0:
            break;
        case 1:
            db1 = src[d_bc++];

            dest[e_bc++] = table.e0.at(db1);
            dest[e_bc++] = table.e1.at((db1 & 0x03) << 4);

            if (padding) {
                dest[e_bc++] = padding_byte;
                dest[e_bc++] = padding_byte;
            }
            break;
        case 2:
            db1 = src[d_bc++];
            db2 = src[d_bc++];

            dest[e_bc++] = table.e0.at(db1);
            dest[e_bc++] = table.e1.at(((db1 & 0x03) << 4) | ((db2 >> 4) & 0x0F));
            dest[e_bc++] = table.e2.at((db2 & 0x0F) << 2);
            
            if (padding) {
                dest[e_bc++] = padding_byte;
            }
            break;
        default:
            break;
        }
    }
};