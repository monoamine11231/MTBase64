/* Special thanks to:
 * https://github.com/client9/stringencoders/blob/master/src/modp_b64_gen.c
 * https://github.com/client9/stringencoders/blob/master/src/modp_b64.c
 */

#include "MTBase64.hpp"

#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <type_traits>

#include <cstdint>
#include <cstring>
#include <cmath>


#include "Implementations/default.cpp"


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



void MTBase64::DecodeMem(uint8_t *dest, const uint8_t *src, std::size_t src_len,
                         const IndexTable& table, bool padding) {

  IndexTableAccessor::DecodeBase64(dest, src, src_len, table, padding);
}


void MTBase64::EncodeMem(uint8_t *dest, const uint8_t *src, std::size_t src_len,
                         const IndexTable& table, bool padding) {

    IndexTableAccessor::EncodeBase64(dest, src, src_len, table, padding);
}

inline bool MTBase64::ValidPaddedEncodedLength(std::size_t encoded_length) {
  return ((encoded_length % 4) == 0) && (encoded_length != 0);
}

inline bool MTBase64::ValidUnpaddedEncodedLength(std::size_t encoded_length) {
  return (encoded_length % 4) != 1 && (encoded_length != 0);
}


/*Cannot calculate real length without knowing if the last 4 bytes consists of
padding or encoded data. Real length is being calculated by
(3 * (encoded_length / 4)) - num_of_padding_chars*/
std::size_t MTBase64::GetDecodedLength(std::size_t encoded_length,
                                       bool padding,
                                       uint8_t padding_num) {

  if (!padding && padding_num > 0)
    throw MTBase64::MTBase64Exception(
      __FILE__, __FUNCTION__, __LINE__,
      MTBase64::ErrorCodeTable::kIllegalFunctionCall,
      "`padding_num` is set when padding is not being used.");

  if (padding && padding_num > 2)
    throw MTBase64::MTBase64Exception(
      __FILE__, __FUNCTION__, __LINE__,
      MTBase64::ErrorCodeTable::kIllegalFunctionCall,
      "Not valid amount of padding is being specified");

  if (padding && !MTBase64::ValidPaddedEncodedLength(encoded_length))
    throw MTBase64::MTBase64Exception(
      __FILE__, __FUNCTION__, __LINE__,
      MTBase64::ErrorCodeTable::kNotValidBase64,
      "Not valid base64 encoding length when padding is being used.");

  if (!padding && !MTBase64::ValidUnpaddedEncodedLength(encoded_length))
    throw MTBase64::MTBase64Exception(
      __FILE__, __FUNCTION__, __LINE__,
      MTBase64::ErrorCodeTable::kNotValidBase64,
      "Not valid base64 encoding length when padding is not being used.");


  /*`fantom_padding_num` is used for base64 encoded data without added padding.
  It describes how much padding is actually needed to for the base64 data
  without added padding. If padding is used, the variable remains 0 */
  uint8_t fantom_padding_num = 0;
  if (!padding && (encoded_length % 4) != 0 )
    fantom_padding_num = 4 - (encoded_length % 4);

  padding_num = (!padding) ? fantom_padding_num : padding_num;


  return 3 * ((encoded_length + fantom_padding_num) / 4) - padding_num;
}

/*Splits the length of uncoded data to whole base64 chunks of 3 characters and
adds 4 bytes of padding if a remainder chunk of 1-2 bytes exist that indicates
a padding at the end*/
std::size_t MTBase64::GetEncodedLength(std::size_t decoded_length,
                                       bool padding) {

  if (padding)
    return ceil((double)decoded_length / 3) * 4;

  return ceil((double)decoded_length * 4 / 3);
}


MTBase64::IndexTable::IndexTable(const std::array<uint8_t, 64>&  linear_table,
                                 uint8_t padding) : e(linear_table),
                                 padding_(padding) {

  std::fill(this->d.begin(), this->d.end(), padding);
  std::fill(this->d0.begin(), this->d0.end(), MTBASE64__BADCHAR);
  std::fill(this->d1.begin(), this->d1.end(), MTBASE64__BADCHAR);
  std::fill(this->d2.begin(), this->d2.end(), MTBASE64__BADCHAR);
  std::fill(this->d3.begin(), this->d3.end(), MTBASE64__BADCHAR);


  for (int i = 0; i < 64; ++i) {
    /*Not allowing padding to be used inside lookup table for preventing
    unexcpected errors and bugs (padding should be a unique character)*/
    if(linear_table.at(i) == padding)
      throw MTBase64::MTBase64Exception(
        __FILE__, __FUNCTION__, __LINE__,
        MTBase64::ErrorCodeTable::kIllegalFunctionCall,
        "Padding shouldn't be used in base64 lookup table.");

    this->e0.at(i*4+0) = linear_table.at(i);
    this->e0.at(i*4+1) = linear_table.at(i);
    this->e0.at(i*4+2) = linear_table.at(i);
    this->e0.at(i*4+3) = linear_table.at(i);

    this->e1.at(i+0*64) = linear_table.at(i);
    this->e1.at(i+1*64) = linear_table.at(i);
    this->e1.at(i+2*64) = linear_table.at(i);
    this->e1.at(i+3*64) = linear_table.at(i);
  
    this->e2.at(i+0*64) = linear_table.at(i);
    this->e2.at(i+1*64) = linear_table.at(i);
    this->e2.at(i+2*64) = linear_table.at(i);
    this->e2.at(i+3*64) = linear_table.at(i);

    /*Most effective way to check for multiple occurences in the index table?
    Checks if the empty slot with a padding value was already set before*/
    if(this->d.at(linear_table.at(i)) != padding)
      throw MTBase64::MTBase64Exception(
        __FILE__, __FUNCTION__, __LINE__,
        MTBase64::ErrorCodeTable::kIllegalFunctionCall,
        "Index table can't have multiple instances of the same character");


    /* Little endian only */
    this->d.at(linear_table.at(i)) = i;
    this->d0.at(linear_table.at(i)) = i << 2;
    this->d1.at(linear_table.at(i)) = ((i & 0x30) >> 4) | ((i & 0x0F) << 12);
    this->d2.at(linear_table.at(i)) = ((i & 0x03) << 22) | ((i & 0x3C) << 6);
    this->d3.at(linear_table.at(i)) = i << 16;
  }
}


uint8_t MTBase64::IndexTable::Lookup(uint8_t index) const {
  return this->e.at(index);
}

uint8_t MTBase64::IndexTable::ReverseLookup(uint8_t index) const {
  uint8_t ret = this->d.at(index);
  if(ret == this->padding_) {
    throw std::out_of_range("Value not in array");
  }
  return ret;
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
