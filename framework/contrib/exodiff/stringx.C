// Copyright(C) 2008 Sandia Corporation.  Under the terms of Contract
// DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains
// certain rights in this software
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
//     * Neither the name of Sandia Corporation nor the names of its
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


#include <cstring>
#include <ctype.h>

#include "stringx.h"
#include "smart_assert.h"

using namespace std;

bool abbreviation( const string & s,
                   const string & master,
                   unsigned min_length )
{
  SMART_ASSERT( min_length > 0 );
  
  if ( s.size() > master.size() )
    return false;

  if ( s.size() < min_length )
    return false;

  for (unsigned i = 0; i < s.size(); ++i)
    if ( s[i] != master[i] )
      return false;
  
  return true;
}

bool no_case_equals( const string & s1, const string & s2 )
{
  if (s1.size() != s2.size()) return false;
  
  for (unsigned i = 0; i < s1.size(); ++i)
    if ( tolower(s1[i]) != tolower(s2[i]) )
      return false;
  
  return true;
}


string & chop_whitespace( string & s )
{
  if (s.size() > 0)
  {
    int i = s.size() - 1;
    for (; i >= 0; --i)
      if ( ! isspace( (int)(s[i]) ) )
        break;
    
    s.resize(i+1);
  }
  
  return s;
}

string extract_token(string & s, const char* delimeters)
{
  if (s.size() > 0)
  {
    SMART_ASSERT( delimeters != 0 && string(delimeters).size() > 0 );
    
    // Move through initial delimeters.
    unsigned p = s.find_first_not_of(delimeters);
    
    if (p >= s.size())
    {
      // no tokens
      s = "";
      return "";
    }
    
    // move to end of first token
    unsigned q = s.find_first_of( delimeters, p );
    
    if (q >= s.size())
    {
      // no more delimeters
      string tok = s.substr( p );
      s = "";
      return tok;
    }
    
    SMART_ASSERT( q > p );
    string tok = s.substr( p, q-p );
    
    // move to start of the second token
    unsigned r = s.find_first_not_of( delimeters, q );
    
    if (r >= s.size())
    {
      // no second token
      s = "";
    }
    else
      s.erase( 0, r );
    
    return tok;
  }
  
  return "";
}

int count_tokens(const string & s, const char* delimeters)
{
  if (!s.empty())
  {
    const char* str_ptr = s.c_str();
    
    // Move through initial delimeters.
    const char* p = &str_ptr[ strspn(str_ptr, delimeters) ];
    
    int num_toks = 0;
    while (p[0] != '\0') {
      ++num_toks;
      p = &p[ strcspn(p, delimeters) ];      // Move through token.
      p = &p[ strspn (p, delimeters) ];      // Move through delimeters.
    }
    
    return num_toks;
  }
  
  return 0;
}


int max_string_length(const vector<string> &names)
{
  if (names.size() == 0)
    return 0;
  unsigned len = names[0].size();
  for (unsigned i=1; i < names.size(); i++) {
    if (names[i].size() > len) {
      len = names[i].size();
    }
  }
  return len;
}

void to_lower( string & s )
{
  for (unsigned i = 0; i < s.size(); ++i)
    s[i] = tolower(s[i]);
}

char first_character( const string & s )
{
  for (unsigned i = 0; i < s.size(); ++i)
    if ( ! isspace( (int)(s[i]) ) )
      return s[i];
  
  return 0;
}

int find_string(const vector<string>& lst, const string& s, bool nocase)
{
  if (nocase)
  {
    for (unsigned i = 0; i < lst.size(); ++i)
      if ( no_case_equals( lst[i], s ) )
        return i;
  }
  else
  {
    for (unsigned i = 0; i < lst.size(); ++i)
      if ( lst[i] == s )
        return i;
  }
  
  return -1;
}

