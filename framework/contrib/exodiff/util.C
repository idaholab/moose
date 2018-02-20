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

#include "util.h"
#include <cstring> // for nullptr, memset
#include <iostream>
#include <unistd.h>

char **
get_name_array(int size, int length)
{
  char ** names = nullptr;
  if (size > 0) {
    names = new char *[size];
    for (int i = 0; i < size; i++)
    {
      names[i] = new char[length + 1];
      std::memset(names[i], '\0', length + 1);
    }
  }
  return names;
}

void
free_name_array(char ** names, int size)
{
  for (int i = 0; i < size; i++)
  {
    delete[] names[i];
  }
  delete[] names;
  names = nullptr;
}

namespace
{
bool
term_out()
{
  static bool is_term = isatty(fileno(stdout));
  return is_term;
}

bool
cerr_out()
{
  static bool is_term = isatty(fileno(stderr));
  return is_term;
}
} // namespace

void
ERR_OUT(std::ostringstream & buf)
{
  if (cerr_out())
  {
    std::cerr << trmclr::red << buf.str() << trmclr::normal;
  }
  else
  {
    std::cerr << buf.str();
  }
}

void
DIFF_OUT(std::ostringstream & buf, trmclr::Style color)
{
  if (term_out())
  {
    std::cout << color << buf.str() << '\n' << trmclr::normal;
  }
  else
  {
    std::cout << buf.str() << '\n';
  }
}

void
DIFF_OUT(const char * buf, trmclr::Style color)
{
  if (term_out())
  {
    std::cout << color << buf << '\n' << trmclr::normal;
  }
  else
  {
    std::cout << buf << '\n';
  }
}
