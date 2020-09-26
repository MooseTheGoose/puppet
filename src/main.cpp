#include "puppet_types.hpp"
#include <gmp.h>
#include <curl/curl.h>

size_t write_response(void *ptr, size_t sz, size_t nmemb, string *s) {
  s->append((char *)ptr, sz * nmemb);
  return sz * nmemb;
}

int main(int argc, const char *argv[]) {
  CURL *curl;
  CURLcode res;
  mpz_t n;

  mpz_init(n);

  string s = "";


  curl = curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:4444/status");
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_response);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
  res = curl_easy_perform(curl);

  printf("%s\r\n", s.data());
  curl_easy_cleanup(curl);

  return 0;
}
