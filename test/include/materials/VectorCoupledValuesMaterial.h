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
 *  A material that couples vector variable values and stores them into material properties
 *  This makes sure that everything is properly resized and can be indexed into.
 */
class VectorCoupledValuesMaterial : public Material
{
public:
  static InputParameters validParams();

  VectorCoupledValuesMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  const bool _request_dotdot;

  const VectorVariableValue & _value;
  const ADVectorVariableValue & _ad_value;
  const VectorVariableValue & _dot;
  const VectorVariableValue * const _dot_dot;
  const VariableValue & _dot_du;
  const VariableValue * const _dot_dot_du;

  const std::string & _var_name;
  MaterialProperty<RealVectorValue> & _value_prop;
  ADMaterialProperty<RealVectorValue> & _ad_value_prop;
  MaterialProperty<RealVectorValue> & _dot_prop;
  MaterialProperty<RealVectorValue> & _dot_dot_prop;
  MaterialProperty<Real> & _dot_du_prop;
  MaterialProperty<Real> & _dot_dot_du_prop;
};
