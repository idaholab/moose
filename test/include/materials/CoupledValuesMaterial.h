//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"

/**
 * A material that couples variables values and stores them into material property
 * This makes sure that everything is properly resized and can be indexed into.
 */
class CoupledValuesMaterial : public Material
{
public:
  static InputParameters validParams();

  CoupledValuesMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  const VariableValue & _value;
  const VariableValue & _dot;
  const VariableValue & _dot_dot;
  const VariableValue & _dot_du;
  const VariableValue & _dot_dot_du;

  const std::string & _var_name;
  MaterialProperty<Real> & _value_prop;
  MaterialProperty<Real> & _dot_prop;
  MaterialProperty<Real> & _dot_dot_prop;
  MaterialProperty<Real> & _dot_du_prop;
  MaterialProperty<Real> & _dot_dot_du_prop;
};
