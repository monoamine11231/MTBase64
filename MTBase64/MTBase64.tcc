namespace MTBase64 {
  template<typename C>
  uint8_t GetPaddingNum(const C& data, const IndexTable& table) {

    std::size_t data_size = data.size();
    uint8_t padding = table.GetPadding();

    typedef typename C::value_type T;
    T a = (data_size>0)?data.at(data_size-1):static_cast<T>(table.Lookup(0));
    T b = (data_size>1)?data.at(data_size-2):static_cast<T>(table.Lookup(0));
    T c = (data_size>2)?data.at(data_size-3):static_cast<T>(table.Lookup(0));

    if (c == padding)
      throw MTBase64::MTBase64Exception(
        __FILE__, __FUNCTION__, __LINE__,
        MTBase64::ErrorCodeTable::kNotValidBase64,
        "Not valid base64 containing 3 padding bytes.");

    return (a == padding) + (b == padding);
  }

  template <template <typename> class Container, typename T>
  Container<T> EncodeCTR(const Container<T>& input, const IndexTable& table,
                        bool using_padding) {
    size_t encoded_length = MTBase64::GetEncodedLength(input.size(),
                                                       using_padding);

    std::shared_ptr<uint8_t> encoded_buffer(new uint8_t[encoded_length],
                                            std::default_delete<uint8_t[]>());

    EncodeMem(
      encoded_buffer.get(), reinterpret_cast<const uint8_t *>(input.data()),
      input.size(), table, using_padding);

    Container<T> output(reinterpret_cast<T*>(encoded_buffer.get()),
                        reinterpret_cast<T*>(encoded_buffer.get())+encoded_length);

    return output;
  }

  template <template <typename> class Container, typename T>
  Container<T> DecodeCTR(const Container<T>& input, const IndexTable& table,
                          bool using_padding) {

    if ((using_padding && ((input.size() % 4) != 0)) ||
        (!using_padding && ((input.size() % 4) == 1)))
        throw MTBase64::MTBase64Exception(
          __FILE__, __FUNCTION__, __LINE__,
          MTBase64::ErrorCodeTable::kNotValidBase64,
          "Not valid base64 encoding length");

    uint8_t padding_num = GetPaddingNum(input, table);
    std::size_t decoded_length = GetDecodedLength(input.size(), using_padding,
                                             padding_num);
    std::shared_ptr<uint8_t> decoded_buffer(new uint8_t[decoded_length],
                                             std::default_delete<uint8_t[]>());

    DecodeMem(
      decoded_buffer.get(), reinterpret_cast<const uint8_t *>(input.data()),
      input.size(), table, using_padding);

    Container<T> output(reinterpret_cast<T*>(decoded_buffer.get()),
                        reinterpret_cast<T*>(decoded_buffer.get())+decoded_length);

    return output;
  }
}
