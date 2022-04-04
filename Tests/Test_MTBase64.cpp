/*--------------------------According to Catch2---------------------------------
Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <array>
#include <memory>
#include <string>

#include <cstring>
#include <cstdint>

#include "MTBase64.hpp"




TEST_CASE("Test MTBase64::IndexTable components", "[MTBase64::IndexTable]") {
  std::array<uint8_t, 64> valid_array = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
    0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13,
    0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D,
    0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31,
    0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
    0x3C, 0x40, 0x3E, 0x3F
  };

  /*Note: the last two characters are the same*/
  std::array<uint8_t, 64> unvalid_array = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
    0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13,
    0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D,
    0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31,
    0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
    0x3C, 0x40, 0x3F, 0x3F
  };

  MTBase64::IndexTable test_table_0(valid_array);
  MTBase64::IndexTable test_table_1(valid_array, 0xF0);

  SECTION("Test initialization") {
    /*Check if padding variable was set right*/
    REQUIRE(test_table_0.GetPadding() == 0x3D);
    REQUIRE(test_table_1.GetPadding() == 0xF0);

    /*Check for copying of the Lookup array during initialization*/
    REQUIRE(test_table_0.Lookup(0x3F) == 63);
    REQUIRE(test_table_0.Lookup(0x0B) == 11);
    REQUIRE(test_table_0.Lookup(0x00) == 0);

    /*Check if the reverse lookup table was build successfully*/
    REQUIRE(test_table_0.ReverseLookup(63) == 0x3F);
    REQUIRE(test_table_0.ReverseLookup(12) == 0x0C);
  }

  SECTION("Test exceptions") {
    /*Padding shouldn't be the same as a symbol inside lookup table*/
    REQUIRE_THROWS_AS(MTBase64::IndexTable(valid_array, 0x2E),
                      MTBase64::MTBase64Exception);

    REQUIRE_THROWS_AS(test_table_0.Lookup(64), std::out_of_range);
    REQUIRE_THROWS_AS(test_table_0.ReverseLookup(0x41), std::out_of_range);

    /*Check if exception is raised when multiple instances of same char in
    table*/
    REQUIRE_THROWS_AS(MTBase64::IndexTable(unvalid_array),
                      MTBase64::MTBase64Exception);
  }
}

TEST_CASE("Test MTBase64::MTBase64Exception","[MTBase64::MTBase64Exception]") {
  std::string msg("Testing MTBase64::MTBase64Exception with catch2");
  try{
    throw MTBase64::MTBase64Exception(
      __FILE__, __FUNCTION__, __LINE__,
      MTBase64::ErrorCodeTable::kIllegalFunctionCall,
      msg.c_str());
  } catch(MTBase64::MTBase64Exception& e) {
    std::string exception_msg             = e.what();
    MTBase64::ErrorCodeTable error_code   = e.GetErrorCode();

    REQUIRE(exception_msg == msg);
    REQUIRE(error_code == MTBase64::ErrorCodeTable::kIllegalFunctionCall);
  }
}

TEST_CASE("Test MTBase64::ValidPaddedEncodedLength",
          "[MTBase64::ValidPaddedEncodedLength]") {

  REQUIRE(MTBase64::ValidPaddedEncodedLength(4));
  REQUIRE(MTBase64::ValidPaddedEncodedLength(8));
  REQUIRE(MTBase64::ValidPaddedEncodedLength(12));
  REQUIRE(MTBase64::ValidPaddedEncodedLength(16));

  REQUIRE_FALSE(MTBase64::ValidPaddedEncodedLength(0));
  REQUIRE_FALSE(MTBase64::ValidPaddedEncodedLength(1));
  REQUIRE_FALSE(MTBase64::ValidPaddedEncodedLength(2));
  REQUIRE_FALSE(MTBase64::ValidPaddedEncodedLength(3));
  REQUIRE_FALSE(MTBase64::ValidPaddedEncodedLength(5));
  REQUIRE_FALSE(MTBase64::ValidPaddedEncodedLength(6));
  REQUIRE_FALSE(MTBase64::ValidPaddedEncodedLength(7));
}

TEST_CASE("Test MTBase64::ValidUnpaddedEncodingLength",
          "[MTBase64::ValidUnpaddedEncodingLength]") {

  REQUIRE(MTBase64::ValidUnpaddedEncodingLength(2));
  REQUIRE(MTBase64::ValidUnpaddedEncodingLength(3));
  REQUIRE(MTBase64::ValidUnpaddedEncodingLength(4));

  REQUIRE(MTBase64::ValidUnpaddedEncodingLength(6));
  REQUIRE(MTBase64::ValidUnpaddedEncodingLength(7));
  REQUIRE(MTBase64::ValidUnpaddedEncodingLength(8));

  REQUIRE_FALSE(MTBase64::ValidUnpaddedEncodingLength(0));
  REQUIRE_FALSE(MTBase64::ValidUnpaddedEncodingLength(1));
  REQUIRE_FALSE(MTBase64::ValidUnpaddedEncodingLength(5));
  REQUIRE_FALSE(MTBase64::ValidUnpaddedEncodingLength(9));
}

TEST_CASE("Test MTBase64::GetEncodedLength", "[MTBase64::GetEncodedLength]") {

  SECTION("Test padded encoding length") {
    REQUIRE(MTBase64::GetEncodedLength(1, true) == 4);
    REQUIRE(MTBase64::GetEncodedLength(2, true) == 4);
    REQUIRE(MTBase64::GetEncodedLength(3, true) == 4);
    REQUIRE(MTBase64::GetEncodedLength(4, true) == 8);
    REQUIRE(MTBase64::GetEncodedLength(5, true) == 8);
    REQUIRE(MTBase64::GetEncodedLength(6, true) == 8);
    REQUIRE(MTBase64::GetEncodedLength(7, true) == 12);
    REQUIRE(MTBase64::GetEncodedLength(8, true) == 12);
    REQUIRE(MTBase64::GetEncodedLength(9, true) == 12);
  }

  SECTION("Test unpadded encoding length") {
    REQUIRE(MTBase64::GetEncodedLength(1, false) == 2);
    REQUIRE(MTBase64::GetEncodedLength(2, false) == 3);
    REQUIRE(MTBase64::GetEncodedLength(3, false) == 4);
    REQUIRE(MTBase64::GetEncodedLength(4, false) == 6);
    REQUIRE(MTBase64::GetEncodedLength(5, false) == 7);
    REQUIRE(MTBase64::GetEncodedLength(6, false) == 8);
    REQUIRE(MTBase64::GetEncodedLength(7, false) == 10);
    REQUIRE(MTBase64::GetEncodedLength(8, false) == 11);
    REQUIRE(MTBase64::GetEncodedLength(9, false) == 12);
  }
}

TEST_CASE("Test MTBase64::GetDecodedLength", "[MTBase64::GetDecodedLength]") {
  SECTION("Test exceptions") {
    /*When `padding_num` is set while padding isn't being used,
    kIllegalFunctionCall exception should be raised*/
    REQUIRE_THROWS_AS(MTBase64::GetDecodedLength(0, false, 1),
                      MTBase64::MTBase64Exception);
    /*Padding amount added can't exceed 2*/
    REQUIRE_THROWS_AS(MTBase64::GetDecodedLength(0, true, 3),
                      MTBase64::MTBase64Exception);
    /*When giving an unvalid encoded length with used padding an
    kIllegalFunctionCall exception should be raised*/
    REQUIRE_THROWS_AS(MTBase64::GetDecodedLength(5, true, 0),
                      MTBase64::MTBase64Exception);
    /*When giving an unvalid encoded length with unused padding an
    kIllegalFunctionCall exception should be raised*/
    REQUIRE_THROWS_AS(MTBase64::GetDecodedLength(5, false, 0),
                      MTBase64::MTBase64Exception);
  }

  SECTION("Testing retured decoded length with variating amount of padding") {
    /*Using padding, no padding added, 1 padding added, 2 padding added*/
    REQUIRE(MTBase64::GetDecodedLength(4, true, 0) == 3);
    REQUIRE(MTBase64::GetDecodedLength(4, true, 1) == 2);
    REQUIRE(MTBase64::GetDecodedLength(4, true, 2) == 1);

    REQUIRE(MTBase64::GetDecodedLength(8, true, 0) == 6);
    REQUIRE(MTBase64::GetDecodedLength(8, true, 1) == 5);
    REQUIRE(MTBase64::GetDecodedLength(8, true, 2) == 4);

    REQUIRE(MTBase64::GetDecodedLength(12, true, 0) == 9);
    REQUIRE(MTBase64::GetDecodedLength(12, true, 1) == 8);
    REQUIRE(MTBase64::GetDecodedLength(12, true, 2) == 7);
  }

  /*Not using padding*/
  SECTION("Test returned decoded length without using padding")
  {
    REQUIRE(MTBase64::GetDecodedLength(2, false) == 1);
    REQUIRE(MTBase64::GetDecodedLength(3, false) == 2);
    REQUIRE(MTBase64::GetDecodedLength(4, false) == 3);

    REQUIRE(MTBase64::GetDecodedLength(6, false) == 4);
    REQUIRE(MTBase64::GetDecodedLength(7, false) == 5);
    REQUIRE(MTBase64::GetDecodedLength(8, false) == 6);

    REQUIRE(MTBase64::GetDecodedLength(10, false) == 7);
    REQUIRE(MTBase64::GetDecodedLength(11, false) == 8);
    REQUIRE(MTBase64::GetDecodedLength(12, false) == 9);
  }
}

TEST_CASE("Test MTBase64::EncodeCTR", "[MTBase64::EncodeCTR]") {
  const MTBase64::IndexTable table = MTBase64::kDefaultBase64;

  /*Using padding*/
  SECTION("Test encoding with added padding and std::string container") {
    REQUIRE(
      MTBase64::EncodeCTR(std::string("d"), table, true)
      ==
      std::string("ZA==")
    );

    REQUIRE(
      MTBase64::EncodeCTR(std::string("dg"), table, true)
      ==
      std::string("ZGc=")
    );

    REQUIRE(
      MTBase64::EncodeCTR(std::string("def"), table, true)
      ==
      std::string("ZGVm")
    );

    REQUIRE(
      MTBase64::EncodeCTR(std::string("defh"), table, true)
      ==
      std::string("ZGVmaA==")
    );

    REQUIRE(
      MTBase64::EncodeCTR(std::string("defhi"), table, true)
      ==
      std::string("ZGVmaGk=")
    );

    REQUIRE(
      MTBase64::EncodeCTR(std::string("defhij"), table, true)
      ==
      std::string("ZGVmaGlq")
    );
  }

  /*-----------------------std::vector<char>--------------------------------*/
  SECTION("Test encoding with added padding and std::vector<char> container") {
    REQUIRE(
      MTBase64::EncodeCTR(std::vector<char>({'d'}), table, true)
      ==
      std::vector<char>({'Z', 'A', '=', '='})
    );

    REQUIRE(
      MTBase64::EncodeCTR(std::vector<char>({'d', 'g'}), table, true)
      ==
      std::vector<char>({'Z', 'G', 'c', '='})
    );

    REQUIRE(
      MTBase64::EncodeCTR(std::vector<char>({'d', 'e', 'f'}), table, true)
      ==
      std::vector<char>({'Z', 'G', 'V', 'm'})
    );

    REQUIRE(
      MTBase64::EncodeCTR(std::vector<char>({'d', 'e', 'f', 'h'}), table, true)
      ==
      std::vector<char>({'Z', 'G', 'V', 'm', 'a', 'A', '=', '='})
    );

    REQUIRE(
      MTBase64::EncodeCTR(std::vector<char>({'d', 'e', 'f', 'h', 'i'}), table,
                          true)
      ==
      std::vector<char>({'Z', 'G', 'V', 'm', 'a', 'G', 'k', '='})
    );

    REQUIRE(
      MTBase64::EncodeCTR(std::vector<char>({'d', 'e', 'f', 'h', 'i', 'j'}),
                          table, true)
      ==
      std::vector<char>({'Z', 'G', 'V', 'm', 'a', 'G', 'l', 'q'})
    );
  }

  /*-----------------------std::vector<uint8_t>-------------------------------*/
  SECTION("Test encoding with added padding and std::vector<uint8_t> container") {
    REQUIRE(
      MTBase64::EncodeCTR(std::vector<uint8_t>({'d'}), table, true)
      ==
      std::vector<uint8_t>({'Z', 'A', '=', '='})
    );

    REQUIRE(
      MTBase64::EncodeCTR(std::vector<uint8_t>({'d', 'g'}), table, true)
      ==
      std::vector<uint8_t>({'Z', 'G', 'c', '='})
    );

    REQUIRE(
      MTBase64::EncodeCTR(std::vector<uint8_t>({'d', 'e', 'f'}), table, true)
      ==
      std::vector<uint8_t>({'Z', 'G', 'V', 'm'})
    );

    REQUIRE(
      MTBase64::EncodeCTR(std::vector<uint8_t>({'d', 'e', 'f', 'h'}),table,true)
      ==
      std::vector<uint8_t>({'Z', 'G', 'V', 'm', 'a', 'A', '=', '='})
    );

    REQUIRE(
      MTBase64::EncodeCTR(std::vector<uint8_t>({'d', 'e', 'f', 'h', 'i'}),table,
                          true)
      ==
      std::vector<uint8_t>({'Z', 'G', 'V', 'm', 'a', 'G', 'k', '='})
    );

    REQUIRE(
      MTBase64::EncodeCTR(std::vector<uint8_t>({'d', 'e', 'f', 'h', 'i', 'j'}),
                          table, true)
      ==
      std::vector<uint8_t>({'Z', 'G', 'V', 'm', 'a', 'G', 'l', 'q'})
    );
  }


  /*------------Test unpadded encoding with std::string container-------------*/
  SECTION("Test encoding without adding padding") {
    REQUIRE(
      MTBase64::EncodeCTR(std::string("d"), table, false)
      ==
      std::string("ZA")
    );

    REQUIRE(
      MTBase64::EncodeCTR(std::string("dg"), table, false)
      ==
      std::string("ZGc")
    );

    REQUIRE(
      MTBase64::EncodeCTR(std::string("def"), table, false)
      ==
      std::string("ZGVm")
    );

    REQUIRE(
      MTBase64::EncodeCTR(std::string("defh"), table, false)
      ==
      std::string("ZGVmaA")
    );

    REQUIRE(
      MTBase64::EncodeCTR(std::string("defhi"), table, false)
      ==
      std::string("ZGVmaGk")
    );

    REQUIRE(
      MTBase64::EncodeCTR(std::string("defhij"), table, false)
      ==
      std::string("ZGVmaGlq")
    );
  }

  /*-----------------------std::vector<char>--------------------------------*/
  SECTION("Test encoding without padding and std::vector<char> container") {
    REQUIRE(
      MTBase64::EncodeCTR(std::vector<char>({'d'}), table, false)
      ==
      std::vector<char>({'Z', 'A'})
    );

    REQUIRE(
      MTBase64::EncodeCTR(std::vector<char>({'d', 'g'}), table, false)
      ==
      std::vector<char>({'Z', 'G', 'c'})
    );

    REQUIRE(
      MTBase64::EncodeCTR(std::vector<char>({'d', 'e', 'f'}), table, false)
      ==
      std::vector<char>({'Z', 'G', 'V', 'm'})
    );

    REQUIRE(
      MTBase64::EncodeCTR(std::vector<char>({'d', 'e', 'f', 'h'}), table, false)
      ==
      std::vector<char>({'Z', 'G', 'V', 'm', 'a', 'A'})
    );

    REQUIRE(
      MTBase64::EncodeCTR(std::vector<char>({'d', 'e', 'f', 'h', 'i'}), table,
                          false)
      ==
      std::vector<char>({'Z', 'G', 'V', 'm', 'a', 'G', 'k'})
    );

    REQUIRE(
      MTBase64::EncodeCTR(std::vector<char>({'d', 'e', 'f', 'h', 'i', 'j'}),
                          table, false)
      ==
      std::vector<char>({'Z', 'G', 'V', 'm', 'a', 'G', 'l', 'q'})
    );
  }

  /*-----------------------std::vector<uint8_t>-------------------------------*/
  SECTION("Test encoding without padding and std::vector<uint8_t> container") {
    REQUIRE(
      MTBase64::EncodeCTR(std::vector<uint8_t>({'d'}), table, false)
      ==
      std::vector<uint8_t>({'Z', 'A'})
    );

    REQUIRE(
      MTBase64::EncodeCTR(std::vector<uint8_t>({'d', 'g'}), table, false)
      ==
      std::vector<uint8_t>({'Z', 'G', 'c'})
    );

    REQUIRE(
      MTBase64::EncodeCTR(std::vector<uint8_t>({'d', 'e', 'f'}), table, false)
      ==
      std::vector<uint8_t>({'Z', 'G', 'V', 'm'})
    );

    REQUIRE(
      MTBase64::EncodeCTR(std::vector<uint8_t>({'d','e','f','h'}),table,false)
      ==
      std::vector<uint8_t>({'Z', 'G', 'V', 'm', 'a', 'A'})
    );

    REQUIRE(
      MTBase64::EncodeCTR(std::vector<uint8_t>({'d', 'e', 'f', 'h', 'i'}),table,
                          false)
      ==
      std::vector<uint8_t>({'Z', 'G', 'V', 'm', 'a', 'G', 'k'})
    );

    REQUIRE(
      MTBase64::EncodeCTR(std::vector<uint8_t>({'d', 'e', 'f', 'h', 'i', 'j'}),
                          table, false)
      ==
      std::vector<uint8_t>({'Z', 'G', 'V', 'm', 'a', 'G', 'l', 'q'})
    );
  }
}

TEST_CASE("Test MTBase64::DecodeStr", "[MTBase64::DecodeStr]") {
  const MTBase64::IndexTable table = MTBase64::kDefaultBase64;

  SECTION("Test exceptions") {
    /*Check if an exception is being raised when using unvalid encoded base64*/
    REQUIRE_THROWS_AS(MTBase64::DecodeCTR(std::string("?"), table, true),
                      MTBase64::MTBase64Exception);

    REQUIRE_THROWS_AS(MTBase64::DecodeCTR(std::string("?"), table, false),
                      MTBase64::MTBase64Exception);

    /*An exception should be raised from ReverseLookup function because of chars
    not in the currently used index table*/
    REQUIRE_THROWS_AS(MTBase64::DecodeCTR(std::string("\x01\x02\x03\x04"), table,
                                          false), std::out_of_range);
  }

  SECTION("Test decoding with used padding and std::string container") {
    REQUIRE(
      MTBase64::DecodeCTR(std::string("ZA=="), table, true)
      ==
      std::string("d")
    );

    REQUIRE(
      MTBase64::DecodeCTR(std::string("ZGc="), table, true)
      ==
      std::string("dg")
    );

    REQUIRE(
      MTBase64::DecodeCTR(std::string("ZGVm"), table, true)
      ==
      std::string("def")
    );

    REQUIRE(
      MTBase64::DecodeCTR(std::string("ZGVmaA=="), table, true)
      ==
      std::string("defh")
    );

    REQUIRE(
      MTBase64::DecodeCTR(std::string("ZGVmaGk="), table, true)
      ==
      std::string("defhi")
    );

    REQUIRE(
      MTBase64::DecodeCTR(std::string("ZGVmaGlq"), table, true)
      ==
      std::string("defhij")
    );
  }
  /*-----------------------std::vector<char>--------------------------------*/
  SECTION("Test decoding with added padding and std::vector<char> container") {
    REQUIRE(
      MTBase64::DecodeCTR(std::vector<char>({'Z', 'A', '=', '='}), table, true)
      ==
      std::vector<char>({'d'})
    );

    REQUIRE(
      MTBase64::DecodeCTR(std::vector<char>({'Z','G','c','='}), table, true)
      ==
      std::vector<char>({'d','g'})
    );

    REQUIRE(
      MTBase64::DecodeCTR(std::vector<char>({'Z','G','V','m'}), table, true)
      ==
      std::vector<char>({'d','e','f'})
    );

    REQUIRE(
      MTBase64::DecodeCTR(std::vector<char>({'Z','G','V','m','a','A','=','='}),
                          table, true)
      ==
      std::vector<char>({'d','e','f','h'})
    );

    REQUIRE(
      MTBase64::DecodeCTR(std::vector<char>({'Z','G','V','m','a','G','k','='}),
                          table, true)
      ==
      std::vector<char>({'d','e','f','h','i'})
    );

    REQUIRE(
      MTBase64::DecodeCTR(std::vector<char>({'Z','G','V','m','a','G','l','q'}),
                                            table, true)
      ==
      std::vector<char>({'d','e','f','h','i','j'})
    );
  }
  /*-----------------------std::vector<uint8_t>-------------------------------*/
  SECTION("Test decoding with added padding and std::vector<uint8_t> container") {
    REQUIRE(
      MTBase64::DecodeCTR(std::vector<uint8_t>({'Z', 'A', '=', '='}),table,true)
      ==
      std::vector<uint8_t>({'d'})
    );

    REQUIRE(
      MTBase64::DecodeCTR(std::vector<uint8_t>({'Z','G','c','='}), table, true)
      ==
      std::vector<uint8_t>({'d','g'})
    );

    REQUIRE(
      MTBase64::DecodeCTR(std::vector<uint8_t>({'Z','G','V','m'}), table, true)
      ==
      std::vector<uint8_t>({'d','e','f'})
    );

    REQUIRE(
      MTBase64::DecodeCTR(std::vector<uint8_t>({'Z','G','V','m','a','A','=','='}),
                          table, true)
      ==
      std::vector<uint8_t>({'d','e','f','h'})
    );

    REQUIRE(
      MTBase64::DecodeCTR(std::vector<uint8_t>({'Z','G','V','m','a','G','k','='}),
                          table, true)
      ==
      std::vector<uint8_t>({'d','e','f','h','i'})
    );

    REQUIRE(
      MTBase64::DecodeCTR(std::vector<uint8_t>({'Z','G','V','m','a','G','l','q'}),
                          table, true)
      ==
      std::vector<uint8_t>({'d','e','f','h','i','j'})
    );
  }


  SECTION("Test decoding without padding and with std::string container") {
    REQUIRE(
      MTBase64::DecodeCTR(std::string("ZA"), table, false)
      ==
      std::string("d")
    );

    REQUIRE(
      MTBase64::DecodeCTR(std::string("ZGc"), table, false)
      ==
      std::string("dg")
    );

    REQUIRE(
      MTBase64::DecodeCTR(std::string("ZGVm"), table, false)
      ==
      std::string("def")
    );

    REQUIRE(
      MTBase64::DecodeCTR(std::string("ZGVmaA"), table, false)
      ==
      std::string("defh")
    );

    REQUIRE(
      MTBase64::DecodeCTR(std::string("ZGVmaGk"), table, false)
      ==
      std::string("defhi")
    );

    REQUIRE(
      MTBase64::DecodeCTR(std::string("ZGVmaGlq"), table, false)
      ==
      std::string("defhij")
    );
  }
  /*-----------------------std::vector<char>--------------------------------*/
  SECTION("Test decoding without padding and std::vector<char> container") {
    REQUIRE(
      MTBase64::DecodeCTR(std::vector<char>({'Z', 'A'}), table, false)
      ==
      std::vector<char>({'d'})
    );

    REQUIRE(
      MTBase64::DecodeCTR(std::vector<char>({'Z','G','c'}), table, false)
      ==
      std::vector<char>({'d','g'})
    );

    REQUIRE(
      MTBase64::DecodeCTR(std::vector<char>({'Z','G','V','m'}), table, false)
      ==
      std::vector<char>({'d','e','f'})
    );

    REQUIRE(
      MTBase64::DecodeCTR(std::vector<char>({'Z','G','V','m','a','A'}),
                          table, false)
      ==
      std::vector<char>({'d','e','f','h'})
    );

    REQUIRE(
      MTBase64::DecodeCTR(std::vector<char>({'Z','G','V','m','a','G','k'}),
                          table, false)
      ==
      std::vector<char>({'d','e','f','h','i'})
    );

    REQUIRE(
      MTBase64::DecodeCTR(std::vector<char>({'Z','G','V','m','a','G','l','q'}),
                                            table, false)
      ==
      std::vector<char>({'d','e','f','h','i','j'})
    );
  }
  /*-----------------------std::vector<uint8_t>-------------------------------*/
  SECTION("Test decoding without padding and std::vector<uint8_t> container") {
    REQUIRE(
      MTBase64::DecodeCTR(std::vector<uint8_t>({'Z', 'A'}),table,false)
      ==
      std::vector<uint8_t>({'d'})
    );

    REQUIRE(
      MTBase64::DecodeCTR(std::vector<uint8_t>({'Z','G','c'}), table, false)
      ==
      std::vector<uint8_t>({'d','g'})
    );

    REQUIRE(
      MTBase64::DecodeCTR(std::vector<uint8_t>({'Z','G','V','m'}), table, false)
      ==
      std::vector<uint8_t>({'d','e','f'})
    );

    REQUIRE(
      MTBase64::DecodeCTR(std::vector<uint8_t>({'Z','G','V','m','a','A'}),
                          table, false)
      ==
      std::vector<uint8_t>({'d','e','f','h'})
    );

    REQUIRE(
      MTBase64::DecodeCTR(std::vector<uint8_t>({'Z','G','V','m','a','G','k'}),
                          table, false)
      ==
      std::vector<uint8_t>({'d','e','f','h','i'})
    );

    REQUIRE(
      MTBase64::DecodeCTR(std::vector<uint8_t>({'Z','G','V','m','a','G','l','q'}),
                          table, false)
      ==
      std::vector<uint8_t>({'d','e','f','h','i','j'})
    );
  }
}

TEST_CASE("Test MTBase64::EncodeMem", "[MTBase64::EncodeMem]") {
  const MTBase64::IndexTable table = MTBase64::kDefaultBase64;
  std::shared_ptr<uint8_t> dest(new uint8_t[8],
                                std::default_delete<uint8_t[]>());

  SECTION("Test exceptions") {
    /*0 input length = exception*/
    REQUIRE_THROWS_AS(MTBase64::EncodeMem(nullptr, nullptr, 0, table, true),
                      MTBase64::MTBase64Exception);
  }

  SECTION("Test encoding with padding") {
    MTBase64::EncodeMem(dest.get(), reinterpret_cast<const uint8_t*>("d"),
                        1, table, true);
    REQUIRE(std::memcmp(dest.get(),
                        reinterpret_cast<const uint8_t*>("ZA=="),
                        4) == 0);

    MTBase64::EncodeMem(dest.get(), reinterpret_cast<const uint8_t*>("dg"),
                        2, table, true);
    REQUIRE(std::memcmp(dest.get(),
                        reinterpret_cast<const uint8_t*>("ZGc="),
                        4) == 0);

    MTBase64::EncodeMem(dest.get(), reinterpret_cast<const uint8_t*>("def"),
                        3, table, true);
    REQUIRE(std::memcmp(dest.get(),
                        reinterpret_cast<const uint8_t*>("ZGVm"),
                        4) == 0);

    MTBase64::EncodeMem(dest.get(), reinterpret_cast<const uint8_t*>("defh"),
                        4, table, true);
    REQUIRE(std::memcmp(dest.get(),
                        reinterpret_cast<const uint8_t*>("ZGVmaA=="),
                        8) == 0);

    MTBase64::EncodeMem(dest.get(), reinterpret_cast<const uint8_t*>("defhi"),
                        5, table, true);
    REQUIRE(std::memcmp(dest.get(),
                        reinterpret_cast<const uint8_t*>("ZGVmaGk="),
                        8) == 0);

    MTBase64::EncodeMem(dest.get(), reinterpret_cast<const uint8_t*>("defhij"),
                        6, table, true);
    REQUIRE(std::memcmp(dest.get(),
                        reinterpret_cast<const uint8_t*>("ZGVmaGlq"),
                        8) == 0);
  }

  SECTION("Test encoding without padding") {
    MTBase64::EncodeMem(dest.get(), reinterpret_cast<const uint8_t*>("d"),
                        1, table, false);
    REQUIRE(std::memcmp(dest.get(),
                        reinterpret_cast<const uint8_t*>("ZA"),
                        2) == 0);

    MTBase64::EncodeMem(dest.get(), reinterpret_cast<const uint8_t*>("dg"),
                        2, table, false);
    REQUIRE(std::memcmp(dest.get(),
                        reinterpret_cast<const uint8_t*>("ZGc"),
                        3) == 0);

    MTBase64::EncodeMem(dest.get(), reinterpret_cast<const uint8_t*>("def"),
                        3, table, false);
    REQUIRE(std::memcmp(dest.get(),
                        reinterpret_cast<const uint8_t*>("ZGVm"),
                        4) == 0);

    MTBase64::EncodeMem(dest.get(), reinterpret_cast<const uint8_t*>("defh"),
                        4, table, false);
    REQUIRE(std::memcmp(dest.get(),
                        reinterpret_cast<const uint8_t*>("ZGVmaA"),
                        6) == 0);

    MTBase64::EncodeMem(dest.get(), reinterpret_cast<const uint8_t*>("defhi"),
                        5, table, false);
    REQUIRE(std::memcmp(dest.get(),
                        reinterpret_cast<const uint8_t*>("ZGVmaGk"),
                        7) == 0);

    MTBase64::EncodeMem(dest.get(), reinterpret_cast<const uint8_t*>("defhij"),
                        6, table, false);
    REQUIRE(std::memcmp(dest.get(),
                        reinterpret_cast<const uint8_t*>("ZGVmaGlq"),
                        8) == 0);
  }
}

TEST_CASE("Test MTBase64::DecodeMem", "[MTBase64::DecodeMem]") {
  const MTBase64::IndexTable table = MTBase64::kDefaultBase64;
  std::shared_ptr<uint8_t> dest(new uint8_t[6],
                                std::default_delete<uint8_t[]>());

  SECTION("Test exceptions") {
    /*Check for valid encoded buffer length*/
    REQUIRE_THROWS_AS(MTBase64::DecodeMem(nullptr, nullptr, 0, table, true),
                      MTBase64::MTBase64Exception);
    REQUIRE_THROWS_AS(MTBase64::DecodeMem(nullptr, nullptr, 2, table, true),
                      MTBase64::MTBase64Exception);
    REQUIRE_THROWS_AS(MTBase64::DecodeMem(nullptr, nullptr, 1, table, false),
                      MTBase64::MTBase64Exception);

    /*std::out_of_range should be raised when using chars in encoded buffer that
    are not defined in the used index table. Exception is being raised from
    ReverseLookup member function in IndexTable class*/
    REQUIRE_THROWS_AS(
      MTBase64::DecodeMem(dest.get(),
                          reinterpret_cast<const uint8_t*>("\x00\x00=="), 4,
                          table, true), std::out_of_range);
  }

  SECTION("Test decoding with padding") {
    MTBase64::DecodeMem(dest.get(),
                        reinterpret_cast<const uint8_t*>("ZA=="), 4, table,
                        true);
    REQUIRE(std::memcmp(dest.get(),
                        reinterpret_cast<const uint8_t*>("d"),
                        1) == 0);

    MTBase64::DecodeMem(dest.get(),
                        reinterpret_cast<const uint8_t*>("ZGc="), 4, table,
                        true);
    REQUIRE(std::memcmp(dest.get(),
                        reinterpret_cast<const uint8_t*>("dg"),
                        2) == 0);

    MTBase64::DecodeMem(dest.get(),
                        reinterpret_cast<const uint8_t*>("ZGVm"), 4, table,
                        true);
    REQUIRE(std::memcmp(dest.get(),
                        reinterpret_cast<const uint8_t*>("def"),
                        3) == 0);

    MTBase64::DecodeMem(dest.get(),
                        reinterpret_cast<const uint8_t*>("ZGVmaA=="), 8, table,
                        true);
    REQUIRE(std::memcmp(dest.get(),
                        reinterpret_cast<const uint8_t*>("defh"),
                        4) == 0);

    MTBase64::DecodeMem(dest.get(),
                        reinterpret_cast<const uint8_t*>("ZGVmaGk="), 8, table,
                        true);
    REQUIRE(std::memcmp(dest.get(),
                        reinterpret_cast<const uint8_t*>("defhi"),
                        5) == 0);

    MTBase64::DecodeMem(dest.get(),
                        reinterpret_cast<const uint8_t*>("ZGVmaGlq"), 8, table,
                        true);
    REQUIRE(std::memcmp(dest.get(),
                        reinterpret_cast<const uint8_t*>("defhij"),
                        6) == 0);
  }

  SECTION("Test decoding without padding") {
    MTBase64::DecodeMem(dest.get(),
                        reinterpret_cast<const uint8_t*>("ZA"), 2, table,
                        false);
    REQUIRE(std::memcmp(dest.get(),
                        reinterpret_cast<const uint8_t*>("d"),
                        1) == 0);

    MTBase64::DecodeMem(dest.get(),
                        reinterpret_cast<const uint8_t*>("ZGc"), 3, table,
                        false);
    REQUIRE(std::memcmp(dest.get(),
                        reinterpret_cast<const uint8_t*>("dg"),
                        2) == 0);

    MTBase64::DecodeMem(dest.get(),
                        reinterpret_cast<const uint8_t*>("ZGVm"), 4, table,
                        false);
    REQUIRE(std::memcmp(dest.get(),
                        reinterpret_cast<const uint8_t*>("def"),
                        3) == 0);

    MTBase64::DecodeMem(dest.get(),
                        reinterpret_cast<const uint8_t*>("ZGVmaA"), 6, table,
                        false);
    REQUIRE(std::memcmp(dest.get(),
                        reinterpret_cast<const uint8_t*>("defh"),
                        4) == 0);

    MTBase64::DecodeMem(dest.get(),
                        reinterpret_cast<const uint8_t*>("ZGVmaGk"), 7, table,
                        false);
    REQUIRE(std::memcmp(dest.get(),
                        reinterpret_cast<const uint8_t*>("defhi"),
                        5) == 0);

    MTBase64::DecodeMem(dest.get(),
                        reinterpret_cast<const uint8_t*>("ZGVmaGlq"), 8, table,
                        false);
    REQUIRE(std::memcmp(dest.get(),
                        reinterpret_cast<const uint8_t*>("defhij"),
                        6) == 0);
  }
}