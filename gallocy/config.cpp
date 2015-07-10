#include "config.h"


/**
 * Read the file at path.
 *
 * :param path: The path to the file to read.
 * :return: The entire file as a string.
 */
gallocy::string read_file(const char* path) {
  std::ifstream t(path, std::ifstream::in);
  gallocy::string s((std::istreambuf_iterator<char>(t)),
                     std::istreambuf_iterator<char>());
  return s;
}


/**
 *  Load a configuration file.
 */
void read_config(gallocy::string &me, peer_list_t &peers) {

  const int buf_sz = 256;

  char parser_buf[buf_sz] = {0};
  const char* path = getenv("GALLOCY_CONFIG");
  int idx = 0;
  struct json_token *arr, *tok;

  gallocy::string contents;

  if (path == NULL)
    contents = read_file("config.json");
  else
    contents = read_file(path);

  // Parse the configuration file
  arr = parse_json2(contents.c_str(), strlen(contents.c_str()));

  // Extract the "me" value
  tok = find_json_token(arr, "me");
  memcpy(parser_buf, tok->ptr, tok->len);
  me = parser_buf;
  memset(parser_buf, 0, buf_sz);

  // Extract the "peers" list value
  tok = find_json_token(arr, "peer[0]");
  while (tok != NULL) {
    memcpy(parser_buf, tok->ptr, tok->len);
    peers.push_back(parser_buf);
    memset(parser_buf, 0, buf_sz);
    idx++;

    gallocy::stringstream s;
    s << "peer[" << idx << "]";
    tok = find_json_token(arr, s.str().c_str());
  }

}
