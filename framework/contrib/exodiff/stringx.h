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

#ifndef STRINGX_H
#define STRINGX_H

#include <string>
#include <vector>

//! Compare a string against another "master" string, where the string, str,
//! can be abbreiviated to as little as min_length characters.  Returns true
//! only if str has at least min_length characters and those that it does
//! have match the master string exactly.
bool abbreviation( const std::string & str,
                   const std::string & master,
                   unsigned min_length );

//! Compares two string ignoring letter case.  Returns true if they are equal.
bool no_case_equals( const std::string & s1, const std::string & s2 );

//! Removes whitespace from the end of the string.  Returns the string given
//! as the argument.
std::string & chop_whitespace( std::string & s );

//! Separates the next token from the given string.  The next token is
//! returned and the given string has the next token removed (so it is
//! modified in place).
std::string extract_token(std::string & s, const char* delimeters = " \t\n\r");

//! Counts how many tokens are contained in the given string.
int count_tokens(const std::string & s, const char* delimeters = " \t\n\r");

//! Runs each string in the vector and returns the maximun size.
int max_string_length(const std::vector<std::string> &names);

//! Replaces each character of the string with its lower case equivalent.
void to_lower( std::string & s );

//! Returns the first non-white space character of the string.  If the string
//! is empty or all white space, a NULL char is returned.
char first_character( const std::string & s );

//! Searches the list of strings for a particular string value.  Letter case
//! will be ignored if the last argument is true.  Returns the index of the
//! string in the vector if found, otherwise returns -1.
int find_string( const std::vector<std::string>& lst,
                 const std::string& s, bool nocase );


#endif
