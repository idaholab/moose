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
class SalehaniIrani3DCTraction;
template <>
InputParameters validParams<SalehaniIrani3DCTraction>();
/**
 * Implementation of the non-stateful exponential traction separation law
 * proposed by Salehani, Mohsen Khajeh and Irani, Nilgoon 2018
 **/
class SalehaniIrani3DCTraction : public CZMMaterialBase
{
public:
  SalehaniIrani3DCTraction(const InputParameters & parameters);

protected:
  virtual RealVectorValue computeTraction() override;

  virtual RankTwoTensor computeTractionDerivatives() override;

  /// the displacement jump associated to the maximum traction
  const std::vector<Real> _delta_u0;

  /// the vector representing the maximum allowed traction in each direction
  const std::vector<Real> _max_allowable_traction;
};
