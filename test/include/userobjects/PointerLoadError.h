//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"

class TypeWithNoLoad
{
public:
  int _i;
};

/// Store but no Load!
template <>
inline void
dataStore(std::ostream & stream, TypeWithNoLoad *& v, void * context)
{
  dataStore(stream, v->_i, context);
}

class PointerLoadError : public GeneralUserObject
{
public:
  static InputParameters validParams();

  PointerLoadError(const InputParameters & params);
  virtual ~PointerLoadError();

  virtual void initialSetup();
  virtual void timestepSetup();

  virtual void initialize(){};
  virtual void execute();
  virtual void finalize(){};

protected:
  TypeWithNoLoad *& _pointer_data;
};
