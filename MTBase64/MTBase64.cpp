#include "MTBase64.hpp"

#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <type_traits>

#include <cstdint>
#include <cstring>
#include <cmath>




const MTBase64::IndexTable MTBase64::kDefaultBase64 = MTBase64::IndexTable({
  'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
  'P','Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d',
  'e','f','g','h','i','j','k','l','m','n','o','p','q','r','s',
  't','u','v','w','x','y','z','0','1','2','3','4','5','6','7',
  '8','9','+','/'});

const MTBase64::IndexTable MTBase64::kUrlSafeBase64 = MTBase64::IndexTable({
  'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
  'P','Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d',
  'e','f','g','h','i','j','k','l','m','n','o','p','q','r','s',
  't','u','v','w','x','y','z','0','1','2','3','4','5','6','7',
  '8','9','-','_'});



/*Reverse chunking of four 6 bit bytes to three 8 bit bytes by using reverse
lookup table and padding checking*/
static void DecodeBase64(uint8_t *dest, const uint8_t *src, const size_t src_len,
                         const MTBase64::IndexTable& table,
                         bool using_padding = true) {

  if (using_padding && !MTBase64::ValidPaddedEncodedLength(src_len))
    throw MTBase64::MTBase64Exception(
      __FILE__, __FUNCTION__, __LINE__,
      MTBase64::ErrorCodeTable::kNotValidBase64,
      "Not valid base64 encoding length when padding is being used.");

  if (!using_padding && !MTBase64::ValidUnpaddedEncodingLength(src_len))
    throw MTBase64::MTBase64Exception(
      __FILE__, __FUNCTION__, __LINE__,
      MTBase64::ErrorCodeTable::kNotValidBase64,
      "Not valid base64 encoding length when padding is not being used.");

  uint8_t padding = table.GetPadding();
  for (size_t e_bc = 0, d_bc = 0; e_bc < src_len;) {

    /*Padding is being used as end of data identifier even when
    `using_padding = false`*/
    uint8_t eb1 = src[e_bc++];
    uint8_t eb2 = src[e_bc++];
    uint8_t eb3 = (e_bc == src_len) ? padding : src[e_bc++];
    uint8_t eb4 = (e_bc == src_len) ? padding : src[e_bc++];

    /*Low level stuff */
    uint8_t to_add = 3 - (eb4 == padding) - (eb3 == padding);
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
static void EncodeBase64(uint8_t *dest, const uint8_t *src, const size_t src_len,
                         const MTBase64::IndexTable& table,
                         bool using_padding = true) {

  if(src_len == 0)
    throw MTBase64::MTBase64Exception(
      __FILE__, __FUNCTION__, __LINE__,
      MTBase64::ErrorCodeTable::kIllegalFunctionCall,
      "input buffer length is 0.");

  uint8_t padding   =  table.GetPadding(),  remainder = src_len % 3;
  size_t d_bc       = 0,                    e_bc      = 0;

  const uint8_t remainder2padding[] = {0, 2, 1};

  /*The loop bellow encodes only whole chunks of 3 bytes*/
  while (d_bc < src_len - remainder) {
    uint8_t db1 = src[d_bc++];
    uint8_t db2 = src[d_bc++];
    uint8_t db3 = src[d_bc++];

    dest[e_bc++] = table.Lookup(db1 >> 2);
    dest[e_bc++] = table.Lookup(((db1 << 4) | (db2 >> 4)) & 0x3f);
    dest[e_bc++] = table.Lookup(((db2 << 2) | (db3 >> 6)) & 0x3f);
    dest[e_bc++] = table.Lookup(db3 & 0x3f);
  }

  if (remainder == 0)  return;
  /*Rest chunk of 1-2 bytes is being encoded bellow*/
  uint8_t r_b1 = src[d_bc++];
  uint8_t r_b2 = (remainder > 1) ? src[d_bc++] : 0x00;

  dest[e_bc++] = table.Lookup(r_b1 >> 2);
  dest[e_bc++] = table.Lookup(((r_b1 << 4) | (r_b2 >> 4)) & 0x3f);

  if (remainder == 2) {
    dest[e_bc++] = table.Lookup((r_b2 << 2) & 0x3f);
  }

  if (!using_padding)  return;

  /*Calculates the amount of padding that is needed from the lookup table*/
  for (uint8_t pad_c = 0; pad_c < remainder2padding[remainder]; pad_c++)
    dest[e_bc++] = padding;

}

void MTBase64::DecodeMem(uint8_t *dest, const uint8_t *src,
                         const size_t src_len,
                         const IndexTable& table, bool using_padding) {

  DecodeBase64(dest, src, src_len, table, using_padding);
}


void MTBase64::EncodeMem(uint8_t *dest, const uint8_t *src,
                         const size_t src_len,
                         const IndexTable& table, bool using_padding) {

    EncodeBase64(dest, src, src_len, table, using_padding);
}

inline bool MTBase64::ValidPaddedEncodedLength(std::size_t encoded_length) {
  return ((encoded_length % 4) == 0) && (encoded_length != 0);
}

inline bool MTBase64::ValidUnpaddedEncodingLength(std::size_t encoded_length) {
  return (encoded_length % 4) != 1 && (encoded_length != 0);
}

/*Cannot calculate real length without knowing if the last 4 bytes consists of
padding or encoded data. Real length is being calculated by
(3 * (encoded_length / 4)) - num_of_padding_chars*/
std::size_t MTBase64::GetDecodedLength(std::size_t encoded_length,
                                       bool using_padding,
                                       uint8_t padding_num) {

  if (!using_padding && padding_num > 0)
    throw MTBase64::MTBase64Exception(
      __FILE__, __FUNCTION__, __LINE__,
      MTBase64::ErrorCodeTable::kIllegalFunctionCall,
      "`padding_num` is set when padding is not being used.");

  if (using_padding && padding_num > 2)
    throw MTBase64::MTBase64Exception(
      __FILE__, __FUNCTION__, __LINE__,
      MTBase64::ErrorCodeTable::kIllegalFunctionCall,
      "Not valid amount of padding is being specified");

  if (using_padding && !MTBase64::ValidPaddedEncodedLength(encoded_length))
    throw MTBase64::MTBase64Exception(
      __FILE__, __FUNCTION__, __LINE__,
      MTBase64::ErrorCodeTable::kNotValidBase64,
      "Not valid base64 encoding length when padding is being used.");

  if (!using_padding && !MTBase64::ValidUnpaddedEncodingLength(encoded_length))
    throw MTBase64::MTBase64Exception(
      __FILE__, __FUNCTION__, __LINE__,
      MTBase64::ErrorCodeTable::kNotValidBase64,
      "Not valid base64 encoding length when padding is not being used.");


  /*`fantom_padding_num` is used for base64 encoded data without added padding.
  It describes how much padding is actually needed to for the base64 data
  without added padding. If padding is used, the variable remains 0 */
  uint8_t fantom_padding_num = 0;
  if (!using_padding && (encoded_length % 4) != 0 )
    fantom_padding_num = 4 - (encoded_length % 4);

  padding_num = (!using_padding) ? fantom_padding_num : padding_num;


  return 3 * ((encoded_length + fantom_padding_num) / 4) - padding_num;
}

/*Splits the length of uncoded data to whole base64 chunks of 3 characters and
adds 4 bytes of padding if a remainder chunk of 1-2 bytes exist that indicates
a padding at the end*/
std::size_t MTBase64::GetEncodedLength(std::size_t decoded_length,
                                       bool using_padding) {

  if (using_padding)
    return ceil((double)decoded_length / 3) * 4;

  return ceil((double)decoded_length * 4 / 3);
}



MTBase64::IndexTable::IndexTable(const std::array<uint8_t, 64>&  linear_table,
                                 uint8_t padding) : table_(linear_table),
                                 padding_(padding) {

  /*Padding should not be used inside the index table and should be a unique
  character, this allows the padding char to be used as an identifier for empty
  slots in the reverse lookup table*/
  std::fill(this->rw_table_.begin(), this->rw_table_.end(), -1);
  for(uint8_t i = 0; i < 64; i++) {
    /*Not allowing padding to be used inside lookup table for preventing
    unexcpected errors and bugs (padding should be a unique character)*/
    if(linear_table.at(i) == padding)
      throw MTBase64::MTBase64Exception(
        __FILE__, __FUNCTION__, __LINE__,
        MTBase64::ErrorCodeTable::kIllegalFunctionCall,
        "Padding shouldn't be used in base64 lookup table.");

    /*Most effective way to check for multiple occurences in the index table?
    Checks if the empty slot with a padding value was already set before*/
    int16_t& to_set = this->rw_table_.at(linear_table[i]);
    if(to_set != -1)
      throw MTBase64::MTBase64Exception(
        __FILE__, __FUNCTION__, __LINE__,
        MTBase64::ErrorCodeTable::kIllegalFunctionCall,
        "Index table can't have multiple instances of the same character");
    to_set = i;
  }
}


uint8_t MTBase64::IndexTable::Lookup(uint8_t index) const {
  return this->table_.at(index);
}

uint8_t MTBase64::IndexTable::ReverseLookup(uint8_t id) const {
  int16_t ret = this->rw_table_.at(id);
  if(ret == -1) {
    throw std::out_of_range("Value not in array");
  }
  return (uint8_t)ret;
}

uint8_t MTBase64::IndexTable::GetPadding() const { return this->padding_; }



MTBase64::MTBase64Exception::MTBase64Exception(const char *file,
                                               const char *function,
                                               std::size_t line_num,
                                               MTBase64::ErrorCodeTable error_code,
                                               const char *error_message) {

  this->file_           = file;
  this->function_       = function;
  this->line_num_       = line_num;

  this->error_code_     = error_code;
  this->error_message_  = error_message;

#if MTBASE64_DEBUG__
  std::cout << file << ": In function `" << function << "`:" << std::endl;
  std::cout << file << ":" << line_num << ": error " << (int)error_code << ": ";
  std::cout << error_message << std::endl;
#endif
}

const char *MTBase64::MTBase64Exception::what() const noexcept {
  return this->error_message_;
}

MTBase64::ErrorCodeTable MTBase64::MTBase64Exception::GetErrorCode() const noexcept
{
  return this->error_code_;
}
