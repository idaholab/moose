// Copyright(C) 2008-2017 National Technology & Engineering Solutions
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
//
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

#include "smart_assert.h" // for SMART_ASSERT
#include "stringx.h"
#include <cctype>  // for tolower, isspace
#include <cstring> // for strspn, strcspn
#include <string>  // for string, operator==
#include <vector>  // for vector

bool
abbreviation(const std::string & s, const std::string & master, unsigned min_length)
{
  SMART_ASSERT(min_length > 0);

  if (s.size() > master.size())
  {
    return false;
  }

  if (s.size() < min_length)
  {
    return false;
  }

  for (unsigned i = 0; i < s.size(); ++i)
  {
    if (s[i] != master[i])
    {
      return false;
    }
  }
  return true;
}

bool
no_case_equals(const std::string & s1, const std::string & s2)
{
  if (s1.size() != s2.size())
  {
    return false;
  }

  for (unsigned i = 0; i < s1.size(); ++i)
  {
    if (tolower(s1[i]) != tolower(s2[i]))
    {
      return false;
    }
  }
  return true;
}

std::string &
chop_whitespace(std::string & s)
{
  if (!s.empty())
  {
    int i = s.size() - 1;
    for (; i >= 0; --i)
    {
      if (isspace(static_cast<int>(s[i])) == 0)
      {
        break;
      }

      s.resize(i + 1);
    }
  }
  return s;
}

std::string
extract_token(std::string & s, const char * delimiters)
{
  if (!s.empty())
  {
    SMART_ASSERT(delimiters != nullptr && !std::string(delimiters).empty());

    // Move through initial delimiters.
    unsigned p = s.find_first_not_of(delimiters);

    if (p >= s.size())
    {
      // no tokens
      s = "";
      return "";
    }

    // move to end of first token
    unsigned q = s.find_first_of(delimiters, p);

    if (q >= s.size())
    {
      // no more delimiters
      std::string tok = s.substr(p);
      s = "";
      return tok;
    }

    SMART_ASSERT(q > p);
    std::string tok = s.substr(p, q - p);

    // move to start of the second token
    unsigned r = s.find_first_not_of(delimiters, q);

    if (r >= s.size())
    {
      // no second token
      s = "";
    }
    else
    {
      s.erase(0, r);
    }

    return tok;
  }

  return "";
}

int
count_tokens(const std::string & s, const char * delimiters)
{
  if (!s.empty())
  {
    const char * str_ptr = s.c_str();

    // Move through initial delimiters.
    const char * p = &str_ptr[strspn(str_ptr, delimiters)];

    int num_toks = 0;
    while (p[0] != '\0') {
      ++num_toks;
      p = &p[strcspn(p, delimiters)]; // Move through token.
      p = &p[strspn(p, delimiters)];  // Move through delimiters.
    }

    return num_toks;
  }

  return 0;
}

int
max_string_length(const std::vector<std::string> & names)
{
  if (names.empty())
  {
    return 0;
  }
  unsigned len = names[0].size();
  for (unsigned i = 1; i < names.size(); i++)
  {
    if (names[i].size() > len) {
      len = names[i].size();
    }
  }
  return len;
}

void
to_lower(std::string & s)
{
  for (auto & elem : s)
  {
    elem = tolower(elem);
  }
}

char
first_character(const std::string & s)
{
  for (auto & elem : s)
  {
    if (isspace(static_cast<int>(elem)) == 0)
    {
      return elem;
    }
  }
  return 0;
}

int
find_string(const std::vector<std::string> & lst, const std::string & s, bool nocase)
{
  if (nocase)
  {
    for (unsigned i = 0; i < lst.size(); ++i)
    {
      if (no_case_equals(lst[i], s))
      {
        return i;
      }
    }
  }
  else
  {
    for (unsigned i = 0; i < lst.size(); ++i)
    {
      if (lst[i] == s)
      {
        return i;
      }
    }
  }
  return -1;
}
