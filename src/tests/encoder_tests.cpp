#include <gmock/gmock.h>

#include <deque>
#include <string>
#include <vector>

#include <process/http.hpp>
#include <process/socket.hpp>

#include <stout/gtest.hpp>

#include "encoder.hpp"
#include "decoder.hpp"

using namespace process;
using namespace process::http;

using std::deque;
using std::string;
using std::vector;


TEST(Encoder, Response)
{
  Request request;
  const OK& response("body");

  // Encode the response.
  const string& encoded = HttpResponseEncoder::encode(response, request);

  // Now decode it back, and verify the encoding was correct.
  ResponseDecoder decoder;
  deque<Response*> responses = decoder.decode(encoded.data(), encoded.length());
  ASSERT_FALSE(decoder.failed());
  ASSERT_EQ(1, responses.size());

  Response* decoded = responses[0];
  EXPECT_EQ("200 OK", decoded->status);
  EXPECT_EQ("body", decoded->body);

  // Encoding should have inserted the 'Date' and 'Content-Length' headers.
  EXPECT_EQ(2, decoded->headers.size());
  EXPECT_TRUE(decoded->headers.contains("Date"));
  EXPECT_SOME_EQ(
      stringify(response.body.size()),
      decoded->headers.get("Content-Length"));
}


TEST(Encoder, AcceptableEncodings)
{
  // Create requests that do not accept gzip encoding.
  vector<Request> requests(7);
  requests[0].headers["Accept-Encoding"] = "gzip;q=0.0,*";
  requests[1].headers["Accept-Encoding"] = "compress";
  requests[2].headers["Accept-Encoding"] = "compress, gzip;q=0.0";
  requests[3].headers["Accept-Encoding"] = "*, gzip;q=0.0";
  requests[4].headers["Accept-Encoding"] = "*;q=0.0, compress";
  requests[5].headers["Accept-Encoding"] = "\n compress";
  requests[6].headers["Accept-Encoding"] = "compress,\tgzip;q=0.0";

  foreach (const Request& request, requests) {
    EXPECT_FALSE(request.accepts("gzip"))
      << "Gzip encoding is unacceptable for 'Accept-Encoding: "
      << request.headers.get("Accept-Encoding").get() << "'";
  }

  // Create requests that accept gzip encoding.
  vector<Request> gzipRequests(12);

  // Using q values.
  gzipRequests[0].headers["Accept-Encoding"] = "gzip;q=0.1,*";
  gzipRequests[1].headers["Accept-Encoding"] = "compress, gzip;q=0.1";
  gzipRequests[2].headers["Accept-Encoding"] = "*, gzip;q=0.5";
  gzipRequests[3].headers["Accept-Encoding"] = "*;q=0.9, compress";
  gzipRequests[4].headers["Accept-Encoding"] = "compress,\tgzip;q=0.1";

  // No q values.
  gzipRequests[5].headers["Accept-Encoding"] = "gzip";
  gzipRequests[6].headers["Accept-Encoding"] = "compress, gzip";
  gzipRequests[7].headers["Accept-Encoding"] = "*";
  gzipRequests[8].headers["Accept-Encoding"] = "*, compress";
  gzipRequests[9].headers["Accept-Encoding"] = "\n gzip";
  gzipRequests[10].headers["Accept-Encoding"] = "compress,\tgzip";
  gzipRequests[11].headers["Accept-Encoding"] = "gzip";

  foreach (const Request& gzipRequest, gzipRequests) {
    EXPECT_TRUE(gzipRequest.accepts("gzip"))
      << "Gzip encoding is acceptable for 'Accept-Encoding: "
      << gzipRequest.headers.get("Accept-Encoding").get() << "'";
  }
}
