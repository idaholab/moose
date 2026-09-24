//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ODETimeDerivative.h"
#include "ADScalarTimeDerivative.h"
#include "FunctorInterface.h"
#include "MooseTypes.h"
#include "SinglePhaseFluidProperties.h"
#include "IncompressibleMomentumSPBase.h"

class SinglePhaseFluidProperties;

template <bool is_ad>
class CoupledPressureIncompressibleMomentumSPScalarKernelTempl
  : public std::conditional<is_ad, ADIncompressibleMomentumSPBase, IncompressibleMomentumSPBase>::
        type
{
  using Base = typename std::
      conditional<is_ad, ADIncompressibleMomentumSPBase, IncompressibleMomentumSPBase>::type;

public:
  CoupledPressureIncompressibleMomentumSPScalarKernelTempl(const InputParameters & parameters);
  virtual bool isADObject() const override { return is_ad; };
  static InputParameters validParams();

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  const VariableValue & _mc;
};

typedef CoupledPressureIncompressibleMomentumSPScalarKernelTempl<false>
    CoupledPressureIncompressibleMomentumSPScalarKernel;
typedef CoupledPressureIncompressibleMomentumSPScalarKernelTempl<true>
    ADCoupledPressureIncompressibleMomentumSPScalarKernel;
