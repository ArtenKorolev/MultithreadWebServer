#pragma once

// necessary on linux
#include <cstdint> // NOLINT
#include <string>
#include <unordered_map>

namespace webserver::http {

using HeadersType = std::unordered_map<std::string, std::string>;

#define LIST_OF_HTTP_STATUS_CODES                                        \
  X(100, CONTINUE, "Continue")                                           \
  X(101, SWITCHING_PROTOCOLS, "Switching Protocols")                     \
  X(102, PROCESSING, "Processing")                                       \
                                                                         \
  X(200, OK, "OK")                                                       \
  X(201, CREATED, "Created")                                             \
  X(202, ACCEPTED, "Accepted")                                           \
  X(203, NON_AUTHORITATIVE_INFORMATION, "Non-Authoritative Information") \
  X(204, NO_CONTENT, "No Content")                                       \
  X(205, RESET_CONTENT, "Reset Content")                                 \
  X(206, PARTIAL_CONTENT, "Partial Content")                             \
                                                                         \
  X(300, MULTIPLE_CHOICES, "Multiple Choices")                           \
  X(301, MOVED_PERMANENTLY, "Moved Permanently")                         \
  X(302, FOUND, "Found")                                                 \
  X(303, SEE_OTHER, "See Other")                                         \
  X(304, NOT_MODIFIED, "Not Modified")                                   \
  X(307, TEMPORARY_REDIRECT, "Temporary Redirect")                       \
  X(308, PERMANENT_REDIRECT, "Permanent Redirect")                       \
                                                                         \
  X(400, BAD_REQUEST, "Bad Request")                                     \
  X(401, UNAUTHORIZED, "Unauthorized")                                   \
  X(402, PAYMENT_REQUIRED, "Payment Required")                           \
  X(403, FORBIDDEN, "Forbidden")                                         \
  X(404, NOT_FOUND, "Not Found")                                         \
  X(405, METHOD_NOT_ALLOWED, "Method Not Allowed")                       \
  X(406, NOT_ACCEPTABLE, "Not Acceptable")                               \
  X(408, REQUEST_TIMEOUT, "Request Timeout")                             \
  X(409, CONFLICT, "Conflict")                                           \
  X(410, GONE, "Gone")                                                   \
  X(411, LENGTH_REQUIRED, "Length Required")                             \
  X(413, PAYLOAD_TOO_LARGE, "Payload Too Large")                         \
  X(414, URI_TOO_LONG, "URI Too Long")                                   \
  X(415, UNSUPPORTED_MEDIA_TYPE, "Unsupported Media Type")               \
  X(429, TOO_MANY_REQUESTS, "Too Many Requests")                         \
                                                                         \
  X(500, INTERNAL_SERVER_ERROR, "Internal Server Error")                 \
  X(501, NOT_IMPLEMENTED, "Not Implemented")                             \
  X(502, BAD_GATEWAY, "Bad Gateway")                                     \
  X(503, SERVICE_UNAVAILABLE, "Service Unavailable")                     \
  X(504, GATEWAY_TIMEOUT, "Gateway Timeout")                             \
  X(505, HTTP_VERSION_NOT_SUPPORTED, "HTTP Version Not Supported")

#define X(numCode, status, reasonPhrase) HTTP_##numCode##_##status = numCode,
enum class StatusCode : std::uint16_t { LIST_OF_HTTP_STATUS_CODES };
#undef X

struct StatusCodeHash {
  std::size_t operator()(StatusCode code) const noexcept {
    return static_cast<std::size_t>(code);
  }
};

using CodeToReasonMapType =
    std::unordered_map<StatusCode, std::string_view, StatusCodeHash>;

#define X(numCode, status, reasonPhrase) \
  {StatusCode::HTTP_##numCode##_##status, reasonPhrase},
inline CodeToReasonMapType getStatusCodeToReasonPhraseMap() {
  static const CodeToReasonMapType statusCodeToReason = {
      LIST_OF_HTTP_STATUS_CODES};

  return statusCodeToReason;
}
#undef X

#define LIST_OF_HTTP_METHODS \
  X(GET)                     \
  X(POST)                    \
  X(HEAD)                    \
  X(OPTIONS)                 \
  X(PATCH)                   \
  X(PUT)                     \
  X(DELETE)

#define X(method) method,
enum class HttpMethod : std::uint8_t { LIST_OF_HTTP_METHODS };
#undef X

using StrToMethodMapType = std::unordered_map<std::string_view, HttpMethod>;

#define X(method) {std::string_view(#method), HttpMethod::method},
inline StrToMethodMapType getStrToMethodMap() {
  static const StrToMethodMapType strToMethod = {LIST_OF_HTTP_METHODS};
  return strToMethod;
}
#undef X

enum class HttpVersion : std::uint8_t {
  HTTP_0_9 = 0,  // HTTP/0.9
  HTTP_1_0,      // HTTP/1.0
  HTTP_1_1,      // HTTP/1.1
  HTTP_2,        // HTTP/2
  HTTP_3,        // HTTP/3
};

}  // namespace webserver::http
