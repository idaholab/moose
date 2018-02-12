#include "STRINGLIB_tokenize.h"
#include <algorithm>

void STRINGLIB::tokenize(const std::string& str, const std::string& separators,
		    std::vector<std::string>& tokens)
{
  std::string curr_token = "";
  for (size_t i = 0; i < str.length(); ++i) {
    char curr_char = str[i];

    // determine if current character is a separator
    bool is_separator = std::find(separators.begin(), separators.end(), curr_char) != separators.end();

    if (is_separator && curr_token != "") {
      // we just completed a token
      tokens.push_back(curr_token);
      curr_token.clear();
    }
    else if (!is_separator) {
      curr_token += curr_char;
    }
  }
  if (curr_token != "") {
    tokens.push_back(curr_token);
  }
}

#if 0
#include <iostream>
using std::cout;
using std::cin;

typedef std::vector<std::string> TokenList;

int main()
{
  char s[128];
  while(!cin.eof()) {
    cout << "Enter a string: ";
    cin.getline(s,128);
    std::string input_line(s);
    if (input_line != "quit") {
      std::vector<std::string> tokens;
      tokenize(input_line, ": \t\r\v\n", tokens);
      cout << "There were " << tokens.size() << " tokens in the line\n";
      TokenList::const_iterator I = tokens.begin();
      while (I != tokens.end()) {
        cout << "'" << *I++ << "'\t";
      }
      cout << '\n';
    } else {
      exit(0);
    }
  }
}
#endif
