#ifndef MTBASE64_HPP
#define MTBASE64_HPP

#include <iostream>
#include <string>
#include <exception>

#include <memory>
#include <array>
#include <vector>

#include <cstdint>

#include <cxxabi.h>
#include <typeinfo>

#include <endian.h>

#if __BYTE_ORDER == __BIG_ENDIAN
	#error "Not implemented for big endian platforms!"
#endif

#define MTBASE64__BADCHAR 0x01FFFFFF

namespace MTBase64 {

enum class ErrorCodeTable
{
  kNotValidBase64,            /*For detecting not valid base64*/
  kIllegalFunctionCall        /*For passing not valid parameters*/
};

class MTBase64Exception : public std::exception
{
private:
  const char *file_;
  const char *function_;
  std::size_t line_num_;

  ErrorCodeTable error_code_;
  const char *error_message_;

public:
  MTBase64Exception(const char *file, const char *function,
                    std::size_t line_num, ErrorCodeTable error_code,
                    const char *error_message);
  virtual ~MTBase64Exception() = default;

  virtual const char *what() const noexcept override;

  ErrorCodeTable GetErrorCode() const noexcept;
};

class IndexTable
{
private:
  std::array<uint32_t, 256> d0;
  std::array<uint32_t, 256> d1;
  std::array<uint32_t, 256> d2;
  std::array<uint32_t, 256> d3;

  std::array<uint32_t, 256> e0;
  std::array<uint32_t, 256> e1;
  std::array<uint32_t, 256> e2;
  

  std::array<uint8_t, 64> e;
  std::array<uint8_t, 256> d;

  uint8_t padding_;

public:
  /*0x3D = '=' unsigned*/
  IndexTable(const std::array<uint8_t, 64>& linear_table, uint8_t padding = 0x3D);

  uint8_t Lookup(uint8_t index) const;
  uint8_t ReverseLookup(uint8_t index) const;

  uint8_t GetPadding() const;

  friend struct IndexTableAccessor;
};

struct IndexTableAccessor;

extern const IndexTable kDefaultBase64;
extern const IndexTable kUrlSafeBase64;

template<typename C>
uint8_t GetPaddingNum(const C& data, const IndexTable& table);

template <template <typename> class Container, typename T>
Container<T> EncodeCTR(const Container<T>& input, const IndexTable& table,
                       bool padding = true);

template <template <typename> class Container, typename T>
Container<T> DecodeCTR(const Container<T>& input, const IndexTable& table,
                       bool padding = true);


bool ValidPaddedEncodedLength(std::size_t encoded_length);
bool ValidUnpaddedEncodedLength(std::size_t encoded_length);

std::size_t GetEncodedLength(std::size_t decoded_length, bool padding);
std::size_t GetDecodedLength(std::size_t encoded_length, bool padding,
                             uint8_t padding_num = 0);

void EncodeMem(uint8_t *dest, const uint8_t *src, std::size_t src_len,
               const IndexTable& table, bool padding = true);
void DecodeMem(uint8_t *dest, const uint8_t *src, std::size_t src_len,
               const IndexTable& table, bool padding = true);

} /* MTBase64 */
/*Import the template implementation file*/
#include "MTBase64.tcc"

#endif /* end of include guard: MTBASE64_HPP */
