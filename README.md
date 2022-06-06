# MTBase64 (Modified Table Base64)
 Custom Table Base64 Encoding and Decoding Library That Supports Encoding and Decoding With and Without Padding.

  MTBase64 works on STL containers such as ```std::vector<T>```, ```std::string```, ```std::basic_string<T>``` and raw C memory. Continuous containers with implemented ```Container<T>.size()``` and ```Container<T>.data()``` functions can be also used with MTBase64.  
<br/>

[![DOCS](https://readthedocs.org/projects/pip/badge/?version=latest&style=flat)](https://github.com/monoamine11231/MTBase64/wiki/DOCS) [![GitHub stars](https://badgen.net/github/stars/monoamine11231/MTBase64/)](https://GitHub.com/monoamine11231/MTBase64/stargazers/) [![Linux](https://svgshare.com/i/Zhy.svg)](https://svgshare.com/i/Zhy.svg) [![contributions welcome](https://img.shields.io/badge/contributions-welcome-brightgreen.svg?style=flat)](https://github.com/monoamine11231/MTBase64/issues)

 ## Why?
 MTBase64 was created to work with Base64 formats that use non-standard Base64 tables and padding without the need of creating new decoders and encoders for a specific Base64 table. MTBase64 makes it possible for the user to choose, use, and create a unique Base64 table for specific purposes.

 ## Dependencies
 * [Ninja](https://ninja-build.org/)
 * C++17

 ## Build and installation
 ```ninja-build``` package needs to be installed for MTBase64 to build.
 MTBase64 can be built using the ```SETUP.sh``` script. All build files can be found in the ```build/``` directory
 * ```./SETUP.sh all``` to build both the static and shared library files
 * ```./SETUP.sh static``` to build the static library file
 * ```./SETUP.sh shared``` to build the shared library file
 * ```./SETUP.sh clean``` to clean all build files


 ```console
 user@linux:~$ git clone "https://github.com/monoamine11231/MTBase64.git"
 user@linux:~$ cd MTBase64/
 user@linux:~/MTBase64$ chmod +x SETUP.sh
 user@linux:~/MTBase64$ ./SETUP.sh X
 ```

## Documentation
[A good start](https://github.com/monoamine11231/MTBase64/wiki/DOCS).

## Usage
When compiling a project with MTBase64, both the ```MTBase64.hpp``` and ```MTBase64.tcc``` files need to be stored in the project's include directory.

```C++
#include <array>
#include <string>
#include <cstdint>

#include "MTBase64.hpp"


/*All functions and classes are stored in the `MTBase64` namespace.
  Use the default Base64 table for encoding and decoding data*/
const MTBase64::IndexTable table1 = MTBase64::kDefaultBase64;


/*The IndexTable can be initialized directly in this way.
 Note that the characters in the array are in the `uint8_t`
 byte form. The array is being converted firstly to std::array
 and then it is being passed to the IndexTable constructor*/
const MTBase64::IndexTable table2 = MTBase64::IndexTable({
  'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
  'P','Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d',
  'e','f','g','h','i','j','k','l','m','n','o','p','q','r','s',
  't','u','v','w','x','y','z','0','1','2','3','4','5','6','7',
  '8','9','+','/'});


/*`MTBase64::EncodeCTR` is used to encode data from STL containers.
  std::string is a container of std::basic_string<char>.
  The function bellow encodes the string "Hello MTBase64!" with the
  default Base64 table defined above and with used padding. The
  decoded container is being returned*/
std::string str_("Hello MTBase64!");
std::string encoded_str = MTBase64::EncodeCTR(str_, table1, true);

/*`MTBase64::DecodeCTR` is used to decode data from STL containers.
  The function above decodes the string with the same default Base64
  table used above and by indicating to the function bellow
  that the data was encoded with padding, using the true boolean.*/
std::string decoded_str = MTBase64::DecodeCTR(encoded_str, table1, true);
```

Run the commands bellow to compile a project that uses MTBase64 with g++
```console
# With shared library file
user@linux:~/Project$ g++ -std=c++17 -L<Path to `libMTBase64.so` file> -I<Path to header files dir> <input files> -o <output> -lMTBase64
# With static library file
user@linux:~/Project$ g++ -std=c++17 -L<Path to `MTBase64.a` file> -I<Path to header files dir> <input files> -o <output> -l:MTBase64.a
```

## Contributing
All contributions are welcome to this project. Feel free to open a pull request where we can discuss the changes to be made.

Don't forget the tests! :)

## Future Ideas
- Optimize with AVX instructions?

## License
This project is licensed under the [Boost Software License 1.0](https://github.com/monoamine11231/MTBase64/blob/main/LICENSE).
