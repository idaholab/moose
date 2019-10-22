//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CZMMaterialBase.h"
class CZM_3DCMaterial;
template <>
InputParameters validParams<CZM_3DCMaterial>();
/**
 *
 */
class CZM_3DCMaterial : public CZMMaterialBase
{
public:
  CZM_3DCMaterial(const InputParameters & parameters);

protected:
  virtual RealVectorValue computeLocalTraction() const override;
  virtual RankTwoTensor computeLocalTractionDerivatives() const override;

  const std::vector<Real> _deltaU0;
  const std::vector<Real> _maxAllowableTraction;
};
