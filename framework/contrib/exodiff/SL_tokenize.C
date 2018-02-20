// Copyright(C) 2009-2017 National Technology & Engineering Solutions
// of Sandia, LLC (NTESS).  Under the terms of Contract DE-NA0003525 with
// NTESS, the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of NTESS nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// (c) Tristan Brindle. Code samples are MIT licensed.
// http://tcbrindle.github.io/posts/a-quicker-study-on-tokenising/

#include "SL_tokenize.h"
#include <algorithm>

std::vector<std::string>
SLIB::tokenize(const std::string & str, const std::string & separators, bool allow_empty_token)
{
  std::vector<std::string> tokens;
  auto first = std::begin(str);
  while (first != std::end(str))
  {
    const auto second =
        std::find_first_of(first, std::end(str), std::begin(separators), std::end(separators));
    if (first != second || allow_empty_token)
    {
      tokens.emplace_back(first, second);
    }
    if (second == std::end(str))
    {
      break;
    }
    first = std::next(second);
  }
  return tokens;
}

#if 0
#include <iostream>

typedef std::vector<std::string> TokenList;

int main()
{
  char s[128];
  while(!std::cin.eof()) {
    std::cout << "Enter a string: ";
    std::cin.getline(s,128);
    std::string input_line(s);
    if (input_line != "quit") {
      std::vector<std::string> tokens = tokenize(input_line, ": \t\r\v\n");
      cout << "There were " << tokens.size() << " tokens in the line\n";
      TokenList::const_iterator I = tokens.begin();
      while (I != tokens.end()) {
	std::cout << "'" << *I++ << "'\t";
      }
      std::cout << '\n';
    } else {
      exit(0);
    }
  }
}
#endif
