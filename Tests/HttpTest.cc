#include <gtest/gtest.h>

#include "HttpParser.h"
#include "HttpResponse.h"

using namespace webserver::http;

TEST(HttpResponseTest, ToStringGeneratesCorrectHttpResponse) {
  HttpResponse response;
  response.httpVersion = "HTTP/1.1";
  response.statusCode = StatusCode::HTTP_200_OK;
  response.headers["Content-Type"] = "text/html";
  response.headers["Content-Length"] = "13";
  response.body = "Hello, world!";

  std::string result = response.toString();

  EXPECT_NE(result.find("HTTP/1.1 200 OK\r\n"), std::string::npos);
  EXPECT_NE(result.find("Content-Type: text/html\r\n"), std::string::npos);
  EXPECT_NE(result.find("Content-Length: 13\r\n"), std::string::npos);
  EXPECT_NE(result.find("\r\n\r\nHello, world!"), std::string::npos);
}

TEST(HttpParserTest, ParsesSimpleGetRequest) {
  const std::string rawRequest =
      "GET /index.html HTTP/1.1\r\n"
      "Host: localhost\r\n"
      "User-Agent: curl/7.68.0\r\n"
      "\r\n";

  const auto [method, httpVersion, uri, headers, body] =
      HttpParser::parse(rawRequest);

  EXPECT_EQ(headers.at("host"), "localhost");
  EXPECT_EQ(headers.at("user-agent"), "curl/7.68.0");
  EXPECT_EQ(method, MethodType::GET);
  EXPECT_EQ(uri, "/index.html");
  EXPECT_EQ(httpVersion, HTTPVersion::HTTP_1_1);
  EXPECT_TRUE(body.empty());
}

TEST(HttpParserTest, ParsesPostRequestWithBody) {
  const std::string rawRequest =
      "POST /submit HTTP/1.1\r\n"
      "Host: localhost\r\n"
      "Content-Length: 11\r\n"
      "Content-Type: text/plain\r\n"
      "\r\n"
      "Hello world";

  const HttpRequest req = HttpParser::parse(rawRequest);

  EXPECT_EQ(req.method, MethodType::POST);
  EXPECT_EQ(req.uri, "/submit");
  EXPECT_EQ(req.httpVersion, HTTPVersion::HTTP_1_1);
  EXPECT_EQ(req.body, "Hello world");
}

TEST(HttpParserTest, ParsesHeadersWithExtraSpaces) {
  const std::string rawRequest =
      "GET /test HTTP/1.1\r\n"
      "Host:    example.com  \r\n"
      "User-Agent: curl/7.68.0 \r\n"
      "\r\n";

  const HttpRequest req = HttpParser::parse(rawRequest);

  EXPECT_EQ(req.method, MethodType::GET);
  EXPECT_EQ(req.uri, "/test");
  EXPECT_EQ(req.httpVersion, HTTPVersion::HTTP_1_1);
  EXPECT_EQ(req.headers.at("host"), "example.com");
}

TEST(HttpParserTest, HandlesLowercaseAndMixedCaseHeaders) {
  const std::string rawRequest =
      "GET / HTTP/1.1\r\n"
      "host: localhost\r\n"
      "Content-Type: text/plain\r\n"
      "\r\n";

  const HttpRequest req = HttpParser::parse(rawRequest);

  EXPECT_EQ(req.headers.at("host"), "localhost");
  EXPECT_EQ(req.headers.at("content-type"), "text/plain");
}

TEST(HttpParserTest, ParsesRequestWithQueryString) {
  const std::string rawRequest =
      "GET /search?q=test HTTP/1.1\r\n"
      "Host: localhost\r\n"
      "\r\n";

  const HttpRequest req = HttpParser::parse(rawRequest);

  EXPECT_EQ(req.uri, "/search?q=test");
}

TEST(HttpParserTest, ParsesPostWithEmptyBody) {
  const std::string rawRequest =
      "POST /submit HTTP/1.1\r\n"
      "Host: localhost\r\n"
      "Content-Length: 0\r\n"
      "\r\n";

  const HttpRequest req = HttpParser::parse(rawRequest);

  EXPECT_TRUE(req.body.empty());
}

TEST(HttpParserTest, HandlesHttp10Request) {
  const std::string rawRequest =
      "GET /old HTTP/1.0\r\n"
      "Host: example.com\r\n"
      "\r\n";

  const HttpRequest req = HttpParser::parse(rawRequest);

  EXPECT_EQ(req.httpVersion, HTTPVersion::HTTP_1_0);
  EXPECT_EQ(req.uri, "/old");
}

TEST(HttpParserTest, ThrowsOnMalformedRequestLine) {
  const std::string rawRequest = "BADREQUEST\r\nHost: x\r\n\r\n";

  EXPECT_THROW(const auto t = HttpParser::parse(rawRequest),
               std::runtime_error);
}

TEST(HttpParserTest, ThrowsOnInvalidMethod) {
  const std::string rawRequest = "FOO / HTTP/1.1\r\nHost: x\r\n\r\n";

  EXPECT_THROW(const auto t = HttpParser::parse(rawRequest), std::out_of_range);
}

// Заголовок без значения
TEST(HttpParserTest, HeaderWithoutValue) {
  const std::string rawRequest =
      "GET / HTTP/1.1\r\n"
      "X-Empty-Header:\r\n"
      "Host: localhost\r\n"
      "\r\n";

  const HttpRequest req = HttpParser::parse(rawRequest);

  EXPECT_EQ(req.headers.at("x-empty-header"), "");
  EXPECT_EQ(req.headers.at("host"), "localhost");
}

// Заголовки с табуляциями
TEST(HttpParserTest, HeaderWithTabsAroundColon) {
  const std::string rawRequest =
      "GET / HTTP/1.1\r\n"
      "Host:   example.com\r\n"
      "User-Agent:\tcurl/7.68.0\r\n"
      "\r\n";

  const HttpRequest req = HttpParser::parse(rawRequest);

  EXPECT_EQ(req.headers.at("host"), "example.com");
  EXPECT_EQ(req.headers.at("user-agent"), "curl/7.68.0");
}

// Очень длинный заголовок
TEST(HttpParserTest, LongHeaderValue) {
  std::string longValue(10000, 'x');
  const std::string rawRequest =
      "GET / HTTP/1.1\r\n"
      "X-Long-Header: " +
      longValue +
      "\r\n"
      "\r\n";

  const HttpRequest req = HttpParser::parse(rawRequest);

  EXPECT_EQ(req.headers.at("x-long-header"), longValue);
}

TEST(HttpParserTest, LastHeaderWithoutTrailingCRLF) {
  const std::string rawRequest =
      "GET / HTTP/1.1\r\n"
      "Host: localhost\r\n"
      "User-Agent: curl/7.68.0\r\n";

  EXPECT_THROW(const auto t = HttpParser::parse(rawRequest),
               std::runtime_error);
}

// Дублирующиеся заголовки
TEST(HttpParserTest, DuplicateHeaders) {
  const std::string rawRequest =
      "GET / HTTP/1.1\r\n"
      "X-Test: one\r\n"
      "X-Test: two\r\n"
      "\r\n";

  const HttpRequest req = HttpParser::parse(rawRequest);

  // В твоей реализации последний должен перезаписать первый
  EXPECT_EQ(req.headers.at("x-test"), "two");
}

// Заголовок с пробелами в имени (некорректный)
TEST(HttpParserTest, HeaderNameWithSpaces) {
  const std::string rawRequest =
      "GET / HTTP/1.1\r\n"
      "X Bad: value\r\n"
      "\r\n";

  EXPECT_THROW(const auto t = HttpParser::parse(rawRequest),
               std::runtime_error);
}

// Заголовок с пустым запросом
TEST(HttpParserTest, EmptyRequest) {
  const std::string rawRequest = "";

  EXPECT_THROW(const auto t = HttpParser::parse(rawRequest),
               std::runtime_error);
}

// Проверка разных кейсов HTTP версии
TEST(HttpParserTest, AllHttpVersions) {
  const std::array<std::string, 5> versions = {"HTTP/0.9", "HTTP/1.0",
                                               "HTTP/1.1", "HTTP/2", "HTTP/3"};
  for (const auto& v : versions) {
    const std::string rawRequest = "GET / " + v + "\r\nHost: localhost\r\n\r\n";
    const HttpRequest req = HttpParser::parse(rawRequest);

    if (v == "HTTP/0.9")
      EXPECT_EQ(req.httpVersion, HTTPVersion::HTTP_0_9);
    else if (v == "HTTP/1.0")
      EXPECT_EQ(req.httpVersion, HTTPVersion::HTTP_1_0);
    else if (v == "HTTP/1.1")
      EXPECT_EQ(req.httpVersion, HTTPVersion::HTTP_1_1);
    else if (v == "HTTP/2")
      EXPECT_EQ(req.httpVersion, HTTPVersion::HTTP_2);
    else if (v == "HTTP/3")
      EXPECT_EQ(req.httpVersion, HTTPVersion::HTTP_3);
  }
}

TEST(HttpParserTest, MalformedHttpVersion) {
  const std::vector<std::string> badVersions = {
      "HTTP/0",  "HTTP/1.",      "HTTP/",     "HTTP/.",
      "HTTP/.1", "HTTP/111.111", "HTTP/1.1.1", "HTTP/ 1.1"};

  for (const auto& v : badVersions) {
    const std::string rawRequest = "GET / " + v + "\r\nHost: localhost\r\n\r\n";
    EXPECT_THROW(const auto t = HttpParser::parse(rawRequest),
                 std::runtime_error);
  }
}

TEST(HttpParserTest, GarbadgeAfterHttpVersion) {
  const std::string rawRequest =
      "GET / HTTP/1.1  df\r\nHost: localhost\r\n\r\n";
  EXPECT_THROW(const auto t = HttpParser::parse(rawRequest),
               std::runtime_error);
}
