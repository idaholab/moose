//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeLagrangianStressPK1.h"

/// Native interface for providing the 2nd Piola Kirchhoff stress
///
/// This class *implements* the 2nd PK stress update, providing:
///   1) The 2nd PK stress
///   2) The derivative of the 2nd PK stress wrt the Cauchy-Green strain
///
/// and wraps these to provide:
///   1) The 1st PK stress
///   2) d(PK1)/d(F)
///
class ComputeLagrangianStressPK2 : public ComputeLagrangianStressPK1
{
public:
  static InputParameters validParams();
  ComputeLagrangianStressPK2(const InputParameters & parameters);

protected:
  /// Wrap PK2 -> PK1
  virtual void computeQpPK1Stress() override;
  /// Provide the PK2 stress and dPK2/dC
  virtual void computeQpPK2Stress() = 0;

protected:
  /// Green-Lagrange strain
  MaterialProperty<RankTwoTensor> & _E;
  /// 2nd PK stress
  MaterialProperty<RankTwoTensor> & _S;
  /// 2nd PK tangent (dS/dF)
  MaterialProperty<RankFourTensor> & _C;
};
