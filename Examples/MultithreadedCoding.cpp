#include <iostream>
#include <fstream>
#include <filesystem>

#include <string>
#include <vector>

#include <memory>

#include <future>

#include <cstdint>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "MTBase64.hpp"



/*Feel free to change, note that `kEReadPerTime` should be a multiple of 3
  while `kDReadPerTime` a multiple of 4*/
const std::size_t kEReadPerTime  = 1024 * 3;
const std::size_t kDReadPerTime  = 1000 * 4;

/*The example script*/
int main(int argc, const char** argv) {
  std::string prog_name(argv[0]);
  if (argc != 4) {
    std::cout << prog_name << ": Not enough arguments" << std::endl;
    return -1;
  }

  /*Read the arguments given*/
  std::string enc_type(argv[1]), ifile_name(argv[2]), ofile_name(argv[3]);

  /*Open the input file as in C unix style for later mapping*/
  int in = open(ifile_name.c_str(), O_RDONLY);
  if (in == -1) {
    std::cout << prog_name << ": Cannot read: " << ifile_name << std::endl;
    return -1;
  }

  /*Create the output file*/
  std::ofstream out(ofile_name, std::ios::binary);
  if(!out.is_open()) {
    std::cout << prog_name << ": Cannot open: " << ofile_name << std::endl;
    return -1;
  }

  std::size_t ifile_size          = std::filesystem::file_size(ifile_name);
  /*Good old file memory mapping*/
  uint8_t* ifile_contents         = static_cast<uint8_t*>(mmap(nullptr,
                                                          ifile_size, PROT_READ,
                                                          MAP_SHARED, in, 0));
  /*Check if mapping the input file was successful*/
  if(ifile_contents == (void*)-1) {
    std::cout << prog_name << ": Cannot map file: " << ifile_name << std::endl;
    return -1;
  }

  /*Async chunks that should be encoded/decoded*/
  std::vector<std::future<void>>  chunk_codecs;

  if (enc_type == "-e") {
    std::size_t encoded_length    = MTBase64::GetEncodedLength(
                                        ifile_size, true);

    /*Create the buffer where all encoded data is going to be stored*/
    uint8_t* encoded_buf          = new uint8_t[encoded_length];
    std::size_t to_read;

    for (std::size_t offset = 0; offset < ifile_size; offset+=kEReadPerTime) {
      /*Check how many bytes of descoded data should be read this time*/
      to_read=(kEReadPerTime+offset>ifile_size)?ifile_size-offset:kEReadPerTime;


      /*Create encoding threads with different read offset from the input file*/
      chunk_codecs.push_back(
        std::async(std::launch::async, MTBase64::EncodeMem,
            encoded_buf + MTBase64::GetEncodedLength(offset, false),
            ifile_contents + offset, to_read,
            std::ref(MTBase64::kDefaultBase64), true));

    }

    /*Wait for all decoding threads to finish*/
    for (auto& handle : chunk_codecs)
      handle.wait();


    /*Write the decoded data to the target output file and delete the buffer*/
    out.write(reinterpret_cast<char*>(encoded_buf), encoded_length);
    delete encoded_buf;

  } else if (enc_type == "-d") {
    /*Get the amount of padding in the input file*/
    uint8_t c1 = ifile_contents[ifile_size - 1];
    uint8_t c2 = ifile_contents[ifile_size - 2];
    uint8_t ifile_padding_num = (c1 == '=') + (c2 == '=');

    /*Estimate the length of the decoded file*/
    std::size_t decoded_length    = MTBase64::GetDecodedLength(
                                        ifile_size, true, ifile_padding_num);

    std::size_t to_read;
    /*Create the target decoded buffer*/
    uint8_t* decoded_buf          = new uint8_t[decoded_length];

    /*The variable names says everything*/
    std::size_t last_chunk_size   = ifile_size % kDReadPerTime;
    std::size_t d_offset_inc = MTBase64::GetDecodedLength(kDReadPerTime, false);
    /*Fancy for loop, increment the offset in the encoded file and in the
      decoded buffer*/
    for (std::size_t offset = 0, d_offset = 0; offset < ifile_size;
      offset += kDReadPerTime, d_offset += d_offset_inc) {

      /*Check how many bytes of encoded data should be read this time*/
      to_read=(offset+kDReadPerTime>ifile_size)?last_chunk_size:kDReadPerTime;

      /*Create async decoding threads for decoding each chunk*/
      chunk_codecs.push_back(
        std::async(std::launch::async, MTBase64::DecodeMem,
          decoded_buf + d_offset,
          ifile_contents + offset, to_read,
          std::ref(MTBase64::kDefaultBase64), true));

    }

    /*Wait for all chunks to be decoded*/
    for (auto& handle : chunk_codecs)
      handle.wait();

    /*std::iostream doesn't like unsigned chars >:( */
    out.write(reinterpret_cast<char*>(decoded_buf), decoded_length);
    /*Free the memory*/
    delete decoded_buf;

  } else {
    /*Ye :/ */
    std::cout << prog_name << ": " << enc_type << " isn't valid" << std::endl;
  }

  /*Close the file and delete the input file mapping*/
  close(in);
  munmap(ifile_contents, ifile_size);
  out.close();
}
