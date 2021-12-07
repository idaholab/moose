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

#include <unordered_map>

template <bool is_ad>
class PiecewiseConstantByBlockMaterialTempl : public Material
{
public:
  PiecewiseConstantByBlockMaterialTempl(const InputParameters & parameters);
  static InputParameters validParams();

protected:
  virtual void computeQpProperties() override;

private:
  GenericMaterialProperty<Real, is_ad> & _prop;
  std::unordered_map<SubdomainID, Real> _sub_id_to_prop;
};

typedef PiecewiseConstantByBlockMaterialTempl<false> PiecewiseConstantByBlockMaterial;
typedef PiecewiseConstantByBlockMaterialTempl<true> ADPiecewiseConstantByBlockMaterial;
