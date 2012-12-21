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
MooseRegEx::search(const std::string & text)
{
  mooseAssert(_re, "regular expression is NULL");

  return trex_search(_re, text.c_str(), NULL, NULL);
}

void
MooseRegEx::cleanUp()
{
  if(_re)
    trex_free(_re);
  _re = NULL;
}
