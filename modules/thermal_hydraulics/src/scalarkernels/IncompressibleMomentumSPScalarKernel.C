//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IncompressibleMomentumSPScalarKernel.h"

#include "FunctorInterface.h"
#include "ScalarCoupleable.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("ThermalHydraulicsApp", IncompressibleMomentumSPScalarKernel);
registerMooseObject("ThermalHydraulicsApp", ADIncompressibleMomentumSPScalarKernel);

template <bool is_ad>
InputParameters
IncompressibleMomentumSPScalarKernelTempl<is_ad>::validParams()
{
  InputParameters params = IncompressibleMomentumSPBaseTempl<is_ad>::validParams();
  params += FunctorInterface::validParams();
  params.addClassDescription(
      "Implements a generic momentum solve over a 1D flow path, acting on the mass flow rate.");
  params.addCoupledVar("reference_pressure_drop",
                       {},
                       "Reference system pressure drop from inlet to outlet. Takes a "
                       "scalar variable name");
  return params;
}

template <bool is_ad>
IncompressibleMomentumSPScalarKernelTempl<is_ad>::IncompressibleMomentumSPScalarKernelTempl(
    const InputParameters & parameters)
  : Base(parameters), _dPc(ScalarCoupleable::coupledScalarValue("reference_pressure_drop"))
{
}

template <bool is_ad>
Real
IncompressibleMomentumSPScalarKernelTempl<is_ad>::computeQpJacobian()
{
  return Base::computeQpJacobianMDot();
}

template class IncompressibleMomentumSPScalarKernelTempl<false>;
template class IncompressibleMomentumSPScalarKernelTempl<true>;
