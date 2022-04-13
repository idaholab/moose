//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "ElementUserObject.h"

class Function;

/**
 * A test object that uses optional material properties
 */
class OptionalTestUserObject : public ElementUserObject
{
public:
  static InputParameters validParams();

  OptionalTestUserObject(const InputParameters & parameters);

  virtual void initialSetup();

  virtual void execute();
  virtual void initialize() {}
  virtual void finalize() {}
  virtual void threadJoin(const UserObject &) {}

private:
  const OptionalMaterialProperty<Real> & _prop;
  const OptionalADMaterialProperty<Real> & _adprop;
  const OptionalMaterialProperty<Real> & _prop_old;
  const OptionalMaterialProperty<Real> & _prop_older;
  const bool _expect;
  const bool _adexpect;
  const bool _expect_old;
  const Function & _func;
  const Function & _adfunc;
};
