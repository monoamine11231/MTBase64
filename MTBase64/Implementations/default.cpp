#include "MTBase64.hpp"


struct MTBase64::IndexTableAccessor {
    /*Reverse chunking of four 6 bit bytes to three 8 bit bytes by using reverse
    lookup table and padding checking*/
    static void DecodeBase64(uint8_t *dest, const uint8_t *src, std::size_t src_len,
                             const MTBase64::IndexTable& table,
                             bool padding = true) {

        /* `src_len == 0` is taken in account in `MTBase64::ValidPaddedEncodedLength`
         * and `MTBase64::ValidUnpaddedEncodedLength`
         */
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
        if (src[src_len-1] != padding_byte && src[src_len-2] == padding_byte)
            throw MTBase64::MTBase64Exception(
            __FILE__, __FUNCTION__, __LINE__,
            MTBase64::ErrorCodeTable::kNotValidBase64,
            "If the penulminate byte is padding, the last one must be padding too.");    

        if (padding) {
            src_len -= src[src_len-1] == padding_byte;
            src_len -= src[src_len-1] == padding_byte;
        }

        /* If the source is % 4 = 0, the last chunk will be treated differently to
         * prevent overflow due to copying an integer holding 3 valid bytes to the
         * destination.
         */
        std::size_t rest = src_len & 3, chunks = (rest == 0) ? src_len / 4 - 1 : src_len / 4;
        std::size_t e_bc = 0, d_bc = 0;
        uint32_t db;
        uint8_t eb0, eb1, eb2, eb3;

        for (std::size_t ch = 0; ch < chunks; ++ch) {
            eb0 = src[e_bc++];
            eb1 = src[e_bc++];
            eb2 = src[e_bc++];
            eb3 = src[e_bc++];

            db = table.d0.at(eb0)|table.d1.at(eb1)|table.d2.at(eb2)|table.d3.at(eb3);
            if (db >= MTBASE64__BADCHAR)
                throw MTBase64::MTBase64Exception(
                __FILE__, __FUNCTION__, __LINE__,
                MTBase64::ErrorCodeTable::kNotValidBase64,
                "Base64 encoded byte was not found in given table during decoding.");
        
            std::memcpy(dest, &db, 4);
            dest += 3;
        }

        switch (rest) {
        case 0: /* We treat the last % 4 = 0 chunk differently to prevent overflow */
            eb0 = src[e_bc++];
            eb1 = src[e_bc++];
            eb2 = src[e_bc++];
            eb3 = src[e_bc++];

            db = table.d0.at(eb0)|table.d1.at(eb1)|table.d2.at(eb2)|table.d3.at(eb3);
            if (db >= MTBASE64__BADCHAR)
                throw MTBase64::MTBase64Exception(
                __FILE__, __FUNCTION__, __LINE__,
                MTBase64::ErrorCodeTable::kNotValidBase64,
                "Base64 encoded byte was not found in given table during decoding.");
            
            /* Prevent overflow on the last chunk */
            std::memcpy(dest, &db, 3);
            break;
        case 1:
            if (padding)
                throw MTBase64::MTBase64Exception(
                __FILE__, __FUNCTION__, __LINE__,
                MTBase64::ErrorCodeTable::kNotValidBase64,
                "Rest of 1 when decoding paddded data. Something bad happened.");
            eb0 = src[e_bc];
            
            db = table.d0.at(eb0);
            if (db >= MTBASE64__BADCHAR)
                throw MTBase64::MTBase64Exception(
                __FILE__, __FUNCTION__, __LINE__,
                MTBase64::ErrorCodeTable::kNotValidBase64,
                "Base64 encoded byte was not found in given table during decoding.");

            std::memcpy(dest, &db, 1);
            break;
        case 2:
            eb0 = src[e_bc++];
            eb1 = src[e_bc++];

            db = table.d0.at(eb0)|table.d1.at(eb1);
            if (db >= MTBASE64__BADCHAR)
                throw MTBase64::MTBase64Exception(
                __FILE__, __FUNCTION__, __LINE__,
                MTBase64::ErrorCodeTable::kNotValidBase64,
                "Base64 encoded byte was not found in given table during decoding.");

            std::memcpy(dest, &db, 1);
            break;
        case 3:
            eb0 = src[e_bc++];
            eb1 = src[e_bc++];
            eb2 = src[e_bc++];

            db = table.d0.at(eb0)|table.d1.at(eb1)|table.d2.at(eb2);
            if (db >= MTBASE64__BADCHAR)
                throw MTBase64::MTBase64Exception(
                __FILE__, __FUNCTION__, __LINE__,
                MTBase64::ErrorCodeTable::kNotValidBase64,
                "Base64 encoded byte was not found in given table during decoding.");

            std::memcpy(dest, &db, 2);
            break;
        default:
            break;
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