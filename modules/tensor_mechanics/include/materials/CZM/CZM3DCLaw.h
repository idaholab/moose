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
class CZM3DCLaw;
template <>
InputParameters validParams<CZM3DCLaw>();
/**
 * Implementation of a simple non-stateful exponential traction separetion law
 * Salehani, Mohsen Khajeh and Irani, Nilgoon 2018
 **/
class CZM3DCLaw : public CZMMaterialBase
{
public:
  CZM3DCLaw(const InputParameters & parameters);

protected:
  virtual RealVectorValue computeTraction() override;
  virtual RankTwoTensor computeTractionDerivatives() override;

  const std::vector<Real> _deltaU0;
  const std::vector<Real> _maxAllowableTraction;
};
