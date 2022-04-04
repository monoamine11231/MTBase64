#include <iostream>
#include <string>

#include "MTBase64.hpp"




int main(int argc, const char **argv) {
  std::string prog_name(argv[0]);
  if (argc != 3) {
    std::cout << prog_name << ": Not enough arguments" << std::endl;
    return -1;
  }

  std::string first_a(argv[1]), second_a(argv[2]);

  /*Encode input */
  if (first_a == "-e") {
    std::cout << MTBase64::EncodeStr(second_a, MTBase64::kDefaultBase64);
    std::cout << std::endl;
    return 0;
  /*Decode input*/
  } else if (first_a == "-d") {
    std::cout << MTBase64::DecodeStr(second_a, MTBase64::kDefaultBase64);
    std::cout << std::endl;
    return 0;
  }

  std::cout << prog_name << ": No matching argument: " << first_a << std::endl;
  return -1;
}
