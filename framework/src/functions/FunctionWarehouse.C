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

#include "FunctionWarehouse.h"

FunctionWarehouse::FunctionWarehouse()
{
}

FunctionWarehouse::~FunctionWarehouse()
{
  for (FunctionIterator i = _functions.begin(); i != _functions.end(); ++i)
    delete i->second;
}

FunctionIterator
FunctionWarehouse::activeFunctionsBegin()
{
  return _functions.begin();
}

FunctionIterator
FunctionWarehouse::activeFunctionsEnd()
{
  return _functions.end();
}

Function &
FunctionWarehouse::getFunction(const std::string & fname)
{
  FunctionIterator iter = _functions.find(fname);

  if (iter == _functions.end())
    mooseError("No Function by name: " + fname);

  return *(iter->second);
}

void
FunctionWarehouse::addFunction(const std::string & fname, Function * func)
{
  _functions[fname] = func;
}
