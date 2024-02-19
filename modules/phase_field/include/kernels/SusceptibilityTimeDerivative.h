//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TimeDerivative.h"
#include "ADTimeDerivative.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

/**
 * This calculates the time derivative for a variable multiplied by a generalized susceptibility
 */

template <bool is_ad>
using SusceptibilityTimeDerivativeBase =
    typename std::conditional<is_ad, ADTimeDerivative, TimeDerivative>::type;

template <bool is_ad>
class SusceptibilityTimeDerivativeTempl
  : public DerivativeMaterialInterface<
        JvarMapKernelInterface<SusceptibilityTimeDerivativeBase<is_ad>>>
{
public:
  static InputParameters validParams();

  SusceptibilityTimeDerivativeTempl(const InputParameters & parameters);

protected:
  /// susceptibility
  const GenericMaterialProperty<Real, is_ad> & _Chi;
};

class SusceptibilityTimeDerivative : public SusceptibilityTimeDerivativeTempl<false>
{
public:
  SusceptibilityTimeDerivative(const InputParameters & parameters);
  virtual void initialSetup() override;

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// susceptibility derivative w.r.t. the kernel variable
  const MaterialProperty<Real> & _dChidu;

  /// susceptibility derivatives w.r.t. coupled variables
  std::vector<const MaterialProperty<Real> *> _dChidarg;
};

class ADSusceptibilityTimeDerivative : public SusceptibilityTimeDerivativeTempl<true>
{
public:
  using SusceptibilityTimeDerivativeTempl<true>::SusceptibilityTimeDerivativeTempl;

protected:
  virtual ADReal precomputeQpResidual() override;
};
