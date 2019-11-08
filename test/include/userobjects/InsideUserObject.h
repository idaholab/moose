//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InternalSideUserObject.h"

class InsideUserObject : public InternalSideUserObject
{
public:
  static InputParameters validParams();

  InsideUserObject(const InputParameters & parameters);
  virtual ~InsideUserObject();

  virtual void initialize();
  virtual void execute();
  virtual void finalize();
  virtual void threadJoin(const UserObject & uo);

  Real getValue() const { return _value; }

protected:
  const VariableValue & _u;
  const VariableValue & _u_neighbor;

  Real _value;
  const MaterialProperty<Real> & _diffusivity_prop;
  const MaterialProperty<Real> & _neighbor_diffusivity_prop;
};
