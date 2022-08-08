//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeLagrangianStressBase.h"

/// Native interface for providing the 1st Piola Kirchhoff stress
///
/// This class *implements* the 1st PK stress update, providing:
///   1) The 1st PK stress
///   2) The derivative of the 1st PK stress wrt the deformation gradient
///
/// and wraps these to provide:
///   1) The Cauchy stress
///   2) The derivative of the increment in the Cauchy stress wrt the
///      increment in the spatial velocity gradient
///
class ComputeLagrangianStressPK1 : public ComputeLagrangianStressBase
{
public:
  static InputParameters validParams();
  ComputeLagrangianStressPK1(const InputParameters & parameters);
  virtual ~ComputeLagrangianStressPK1(){};

protected:
  /// Calculate the stress update to provide both measures (cauchy and pk1)
  virtual void computeQpStressUpdate() override;
  /// Provide for the actual PK stress update (just PK1)
  virtual void computeQpPK1Stress() = 0;

private:
  /// Wrap the PK stress to get the Cauchy stress
  virtual void computeQpCauchyStress();

protected:
  /// Inverse incremental deformation gradient
  const MaterialProperty<RankTwoTensor> & _inv_df;
  /// Deformation gradient
  const MaterialProperty<RankTwoTensor> & _F;
};
