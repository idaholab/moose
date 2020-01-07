//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"
#include "MaterialProperty.h"

/**
 * Implements a simple Neumann BC with material where grad(u)=value on the boundary.
 */
class MTBC : public IntegratedBC
{
public:
  static InputParameters validParams();

  MTBC(const InputParameters & parameters);

  virtual ~MTBC() {}

protected:
  virtual Real computeQpResidual();

private:
  /**
   * Value of grad(u) on the boundary.
   */
  Real _value;
  std::string _prop_name;
  const MaterialProperty<Real> & _mat;
};
