#ifndef __STOUT_GZIP_HPP__
#define __STOUT_GZIP_HPP__

#ifdef HAVE_LIBZ
#include <zlib.h>
#endif

#include <string>

#include "error.hpp"
#include "try.hpp"

// Compression utilities.
// TODO(bmahler): Provide streaming compression / decompression as well.
namespace gzip {

// We use a 16KB buffer with zlib compression / decompression.
#define GZIP_BUFFER_SIZE 16384

// Returns a gzip compressed version of the provided string.
// The compression level should be within the range [-1, 9].
// See zlib.h:
//   #define Z_NO_COMPRESSION         0
//   #define Z_BEST_SPEED             1
//   #define Z_BEST_COMPRESSION       9
//   #define Z_DEFAULT_COMPRESSION  (-1)
inline Try<std::string> compress(
    const std::string& decompressed,
#ifdef HAVE_LIBZ
    int level = Z_DEFAULT_COMPRESSION)
#else
    int level = -1)
#endif
{
#ifndef HAVE_LIBZ
  return Error("libz is not available");
#else
  // Verify the level is within range.
  if (!(level == Z_DEFAULT_COMPRESSION ||
      (level >= Z_NO_COMPRESSION && level <= Z_BEST_COMPRESSION))) {
    return Error("Invalid compression level: " + level);
  }

  z_stream_s stream;
  stream.next_in =
    const_cast<Bytef*>(reinterpret_cast<const Bytef*>(decompressed.data()));
  stream.avail_in = decompressed.length();
  stream.zalloc = Z_NULL;
  stream.zfree = Z_NULL;
  stream.opaque = Z_NULL;

  int code = deflateInit2(
      &stream,
      level,          // Compression level.
      Z_DEFLATED,     // Compression method.
      MAX_WBITS + 16, // Zlib magic for gzip compression / decompression.
      8,              // Default memLevel value.
      Z_DEFAULT_STRATEGY);

  if (code != Z_OK) {
    return Error("Failed to initialize zlib: " + std::string(stream.msg));
  }

  // Build up the compressed result.
  Bytef buffer[GZIP_BUFFER_SIZE];
  std::string result = "";
  do {
    stream.next_out = buffer;
    stream.avail_out = GZIP_BUFFER_SIZE;
    code = deflate(&stream, stream.avail_in > 0 ? Z_NO_FLUSH : Z_FINISH);

    if (code != Z_OK && code != Z_STREAM_END) {
      Error error(std::string(stream.msg));
      deflateEnd(&stream);
      return error;
    }

    // Consume output and reset the buffer.
    result.append(
        reinterpret_cast<char*>(buffer),
        GZIP_BUFFER_SIZE - stream.avail_out);
    stream.next_out = buffer;
    stream.avail_out = GZIP_BUFFER_SIZE;
  } while (code != Z_STREAM_END);

  code = deflateEnd(&stream);
  if (code != Z_OK) {
    return Error("Failed to clean up zlib: " + std::string(stream.msg));
  }
  return result;
#endif // HAVE_LIBZ
}


// Returns a gzip decompressed version of the provided string.
inline Try<std::string> decompress(const std::string& compressed)
{
#ifndef HAVE_LIBZ
  return Error("libz is not available");
#else
  z_stream_s stream;
  stream.next_in =
    const_cast<Bytef*>(reinterpret_cast<const Bytef*>(compressed.data()));
  stream.avail_in = compressed.length();
  stream.zalloc = Z_NULL;
  stream.zfree = Z_NULL;
  stream.opaque = Z_NULL;

  int code = inflateInit2(
      &stream,
      MAX_WBITS + 16); // Zlib magic for gzip compression / decompression.

  if (code != Z_OK) {
    return Error("Failed to initialize zlib: " + std::string(stream.msg));
  }

  // Build up the decompressed result.
  Bytef buffer[GZIP_BUFFER_SIZE];
  std::string result = "";
  do {
    stream.next_out = buffer;
    stream.avail_out = GZIP_BUFFER_SIZE;
    code = inflate(&stream, stream.avail_in > 0 ? Z_NO_FLUSH : Z_FINISH);

    if (code != Z_OK && code != Z_STREAM_END) {
      Error error(std::string(stream.msg));
      inflateEnd(&stream);
      return error;
    }

    // Consume output and reset the buffer.
    result.append(
        reinterpret_cast<char*>(buffer),
        GZIP_BUFFER_SIZE - stream.avail_out);
    stream.next_out = buffer;
    stream.avail_out = GZIP_BUFFER_SIZE;
  } while (code != Z_STREAM_END);

  code = inflateEnd(&stream);
  if (code != Z_OK) {
    return Error("Failed to clean up zlib: " + std::string(stream.msg));
  }
  return result;
#endif // HAVE_LIBZ
}

} // namespace gzip {

#endif // __STOUT_GZIP_HPP__
