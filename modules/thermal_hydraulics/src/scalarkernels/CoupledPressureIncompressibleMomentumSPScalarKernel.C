//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledPressureIncompressibleMomentumSPScalarKernel.h"

#include "FunctorInterface.h"
#include "ScalarCoupleable.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("ThermalHydraulicsApp", CoupledPressureIncompressibleMomentumSPScalarKernel);
registerMooseObject("ThermalHydraulicsApp", ADCoupledPressureIncompressibleMomentumSPScalarKernel);

template <bool is_ad>
InputParameters
CoupledPressureIncompressibleMomentumSPScalarKernelTempl<is_ad>::validParams()
{
  InputParameters params = IncompressibleMomentumSPBaseTempl<is_ad>::validParams();
  params += FunctorInterface::validParams();
  params.addClassDescription("Implements a generic momentum solve over a 1D flow path, acting on "
                             "the reference pressure drop.");
  params.addCoupledVar("coupled_mass_flow_rate",
                       {},
                       "coupled_mass_flow_rate. Takes a "
                       "scalar variable name");

  return params;
}

template <bool is_ad>
CoupledPressureIncompressibleMomentumSPScalarKernelTempl<is_ad>::
    CoupledPressureIncompressibleMomentumSPScalarKernelTempl(const InputParameters & parameters)
  : Base(parameters), _mc(ScalarCoupleable::coupledScalarValue("coupled_mass_flow_rate"))
{
}

template <bool is_ad>
GenericReal<is_ad>
CoupledPressureIncompressibleMomentumSPScalarKernelTempl<is_ad>::massFlowRate()
{
  return _mc[0];
}

template <bool is_ad>
GenericReal<is_ad>
CoupledPressureIncompressibleMomentumSPScalarKernelTempl<is_ad>::pressureDrop()
{
  return -Base::_u[0];
}

template <bool is_ad>
Real
CoupledPressureIncompressibleMomentumSPScalarKernelTempl<is_ad>::computeQpJacobian()
{
  return Base::computeQpJacobianDP();
}

template class CoupledPressureIncompressibleMomentumSPScalarKernelTempl<false>;
template class CoupledPressureIncompressibleMomentumSPScalarKernelTempl<true>;
