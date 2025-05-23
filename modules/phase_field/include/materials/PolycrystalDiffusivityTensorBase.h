//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "DerivativeMaterialInterface.h"
#include "RankThreeTensor.h"

/**
 * Generates a diffusion tensor to distinguish between the bulk, grain boundary,
 * and surface diffusion rates.
 */
class PolycrystalDiffusivityTensorBase : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  PolycrystalDiffusivityTensorBase(const InputParameters & parameters);

protected:
  virtual void computeProperties();
  const VariableValue & _T;
  std::vector<const VariableValue *> _vals;
  std::vector<const VariableGradient *> _grad_vals;
  const VariableValue & _c;
  const VariableGradient & _grad_c;
  VariableName _c_name;

  std::string _diffusivity_name;
  MaterialProperty<RealTensorValue> & _D;
  MaterialProperty<RealTensorValue> * _dDdc;
  MaterialProperty<RankThreeTensor> * _dDdgradc;

  Real _D0;
  Real _Em;
  Real _s_index;
  Real _gb_index;
  Real _b_index;
  Real _Dbulk;

  const Real _kb;
  const unsigned int _op_num;

  /// solid phase order parameters
  std::vector<NonlinearVariableName> _vals_name;
  std::vector<MaterialProperty<RealTensorValue> *> _dDdeta;
};
