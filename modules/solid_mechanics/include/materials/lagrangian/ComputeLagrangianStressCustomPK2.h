//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeLagrangianStressPK1.h"

/// Adapt a custom-defined PK2 stress and its Jacobian to provide the PK1 stress and its Jacobian.
class ComputeLagrangianStressCustomPK2 : public ComputeLagrangianStressPK1
{
public:
  static InputParameters validParams();
  ComputeLagrangianStressCustomPK2(const InputParameters & parameters);

protected:
  void computeQpPK1Stress() override;

protected:
  /// 2nd PK stress
  const MaterialProperty<RankTwoTensor> & _pk2;
  /// 2nd PK tangent (dS/dF)
  const MaterialProperty<RankFourTensor> & _dpk2_dF;
};
