/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "MooseRegEx.h"
#include "MooseError.h"

MooseRegEx::MooseRegEx() :
    _re(NULL)
{
}

MooseRegEx::MooseRegEx(const std::string & pattern) :
    _re(NULL)
{
  compile(pattern);
}

MooseRegEx::~MooseRegEx()
{
  cleanUp();
}

void
MooseRegEx::compile(const std::string & pattern)
{
  const TRexChar *error;
  cleanUp();
  _re = trex_compile(pattern.c_str(), &error);

  if (!_re)
    mooseError(std::string("Error compiling RegEx: ") + error);
}

bool
MooseRegEx::search(const std::string & str) const
{
  mooseAssert(_re, "Regular expression is NULL");

  return trex_search(_re, str.c_str(), NULL, NULL);
}

bool
MooseRegEx::search(const std::string & str, std::vector<std::string> & groups) const
{
  mooseAssert(_re, "Regular expression is NULL");

  groups.clear();

  TRexBool result = trex_search(_re, str.c_str(), NULL, NULL);

  if (result)
  {
    unsigned int count = trex_getsubexpcount(_re);
    groups.resize(count);
    TRexMatch match;

    for (unsigned int i=0; i<count; ++i)
    {
      result = trex_getsubexp(_re, i, &match);
      mooseAssert(result, "Error retrieving subexpression from TRex");

      groups[i] = std::string(match.begin, match.len);
    }
  }
  return result;
}

bool
MooseRegEx::findall(const std::string & str, std::vector<std::string> & groups) const
{
  return findSplit(str, groups, true);
}

bool
MooseRegEx::split(const std::string & str, std::vector<std::string> & groups) const
{
  return findSplit(str, groups, false);
}

bool
MooseRegEx::findSplit(const std::string & str, std::vector<std::string> & groups, bool use_matches) const
{
  mooseAssert(_re, "Regular expression is NULL");

  const TRexChar *in_begin = str.c_str();
  const TRexChar *in_end = in_begin + str.length();

  const TRexChar *out_begin, *out_end;
  TRexBool result, first_result=false;

  groups.clear();
  do
  {
    result = trex_search(_re, in_begin, &out_begin, &out_end);

    if (result)
    {
      first_result = true;

      if (use_matches)
        groups.push_back(std::string(out_begin, out_end-out_begin));
      else if (out_begin-in_begin > 0)
        groups.push_back(std::string(in_begin, out_begin-in_begin));
    }

    in_begin = out_end;
  }
  while (result && in_begin < in_end);

  return first_result;
}

void
MooseRegEx::cleanUp()
{
  if(_re)
    trex_free(_re);
  _re = NULL;
}
