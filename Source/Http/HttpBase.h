#pragma once

#include <string>
#include <unordered_map>

namespace webserver::http {

using HeadersType = std::unordered_map<std::string, std::string>;

enum class StatusCode : std::uint16_t {
  HTTP_100_CONTINUE = 100,
  HTTP_101_SWITCHING_PROTOCOLS = 101,
  HTTP_102_PROCESSING = 102,

  HTTP_200_OK = 200,
  HTTP_201_CREATED = 201,
  HTTP_202_ACCEPTED = 202,
  HTTP_203_NON_AUTHORITATIVE_INFORMATION = 203,
  HTTP_204_NO_CONTENT = 204,
  HTTP_205_RESET_CONTENT = 205,
  HTTP_206_PARTIAL_CONTENT = 206,

  HTTP_300_MULTIPLE_CHOICES = 300,
  HTTP_301_MOVED_PERMANENTLY = 301,
  HTTP_302_FOUND = 302,
  HTTP_303_SEE_OTHER = 303,
  HTTP_304_NOT_MODIFIED = 304,
  HTTP_307_TEMPORARY_REDIRECT = 307,
  HTTP_308_PERMANENT_REDIRECT = 308,

  HTTP_400_BAD_REQUEST = 400,
  HTTP_401_UNAUTHORIZED = 401,
  HTTP_402_PAYMENT_REQUIRED = 402,
  HTTP_403_FORBIDDEN = 403,
  HTTP_404_NOT_FOUND = 404,
  HTTP_405_METHOD_NOT_ALLOWED = 405,
  HTTP_406_NOT_ACCEPTABLE = 406,
  HTTP_408_REQUEST_TIMEOUT = 408,
  HTTP_409_CONFLICT = 409,
  HTTP_410_GONE = 410,
  HTTP_411_LENGTH_REQUIRED = 411,
  HTTP_413_PAYLOAD_TOO_LARGE = 413,
  HTTP_414_URI_TOO_LONG = 414,
  HTTP_415_UNSUPPORTED_MEDIA_TYPE = 415,
  HTTP_429_TOO_MANY_REQUESTS = 429,

  HTTP_500_INTERNAL_SERVER_ERROR = 500,
  HTTP_501_NOT_IMPLEMENTED = 501,
  HTTP_502_BAD_GATEWAY = 502,
  HTTP_503_SERVICE_UNAVAILABLE = 503,
  HTTP_504_GATEWAY_TIMEOUT = 504,
  HTTP_505_HTTP_VERSION_NOT_SUPPORTED = 505
};

enum class MethodType : std::uint8_t {
  GET,
  POST,
  HEAD,
  OPTIONS,
  PATCH,
  PUT,
  DELETE
};

struct StatusCodeHash {
  std::size_t operator()(StatusCode code) const noexcept {
    return static_cast<std::size_t>(code);
  }
};

inline std::unordered_map<StatusCode, std::string_view, StatusCodeHash>
generateStatusCodeMap() {
  static const std::unordered_map<StatusCode, std::string_view, StatusCodeHash>
      statusCodeReasonPhrase = {
          {StatusCode::HTTP_100_CONTINUE, "Continue"},
          {StatusCode::HTTP_101_SWITCHING_PROTOCOLS, "Switching Protocols"},
          {StatusCode::HTTP_102_PROCESSING, "Processing"},

          {StatusCode::HTTP_200_OK, "OK"},
          {StatusCode::HTTP_201_CREATED, "Created"},
          {StatusCode::HTTP_202_ACCEPTED, "Accepted"},
          {StatusCode::HTTP_203_NON_AUTHORITATIVE_INFORMATION,
           "Non-Authoritative Information"},
          {StatusCode::HTTP_204_NO_CONTENT, "No Content"},
          {StatusCode::HTTP_205_RESET_CONTENT, "Reset Content"},
          {StatusCode::HTTP_206_PARTIAL_CONTENT, "Partial Content"},

          {StatusCode::HTTP_300_MULTIPLE_CHOICES, "Multiple Choices"},
          {StatusCode::HTTP_301_MOVED_PERMANENTLY, "Moved Permanently"},
          {StatusCode::HTTP_302_FOUND, "Found"},
          {StatusCode::HTTP_303_SEE_OTHER, "See Other"},
          {StatusCode::HTTP_304_NOT_MODIFIED, "Not Modified"},
          {StatusCode::HTTP_307_TEMPORARY_REDIRECT, "Temporary Redirect"},
          {StatusCode::HTTP_308_PERMANENT_REDIRECT, "Permanent Redirect"},

          {StatusCode::HTTP_400_BAD_REQUEST, "Bad Request"},
          {StatusCode::HTTP_401_UNAUTHORIZED, "Unauthorized"},
          {StatusCode::HTTP_402_PAYMENT_REQUIRED, "Payment Required"},
          {StatusCode::HTTP_403_FORBIDDEN, "Forbidden"},
          {StatusCode::HTTP_404_NOT_FOUND, "Not Found"},
          {StatusCode::HTTP_405_METHOD_NOT_ALLOWED, "Method Not Allowed"},
          {StatusCode::HTTP_406_NOT_ACCEPTABLE, "Not Acceptable"},
          {StatusCode::HTTP_408_REQUEST_TIMEOUT, "Request Timeout"},
          {StatusCode::HTTP_409_CONFLICT, "Conflict"},
          {StatusCode::HTTP_410_GONE, "Gone"},
          {StatusCode::HTTP_411_LENGTH_REQUIRED, "Length Required"},
          {StatusCode::HTTP_413_PAYLOAD_TOO_LARGE, "Payload Too Large"},
          {StatusCode::HTTP_414_URI_TOO_LONG, "URI Too Long"},
          {StatusCode::HTTP_415_UNSUPPORTED_MEDIA_TYPE,
           "Unsupported Media Type"},
          {StatusCode::HTTP_429_TOO_MANY_REQUESTS, "Too Many Requests"},

          {StatusCode::HTTP_500_INTERNAL_SERVER_ERROR, "Internal Server Error"},
          {StatusCode::HTTP_501_NOT_IMPLEMENTED, "Not Implemented"},
          {StatusCode::HTTP_502_BAD_GATEWAY, "Bad Gateway"},
          {StatusCode::HTTP_503_SERVICE_UNAVAILABLE, "Service Unavailable"},
          {StatusCode::HTTP_504_GATEWAY_TIMEOUT, "Gateway Timeout"},
          {StatusCode::HTTP_505_HTTP_VERSION_NOT_SUPPORTED,
           "HTTP Version Not Supported"},
      };

  return statusCodeReasonPhrase;
}
}  // namespace webserver::http
