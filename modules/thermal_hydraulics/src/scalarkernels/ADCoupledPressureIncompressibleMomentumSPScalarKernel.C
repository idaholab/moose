//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCoupledPressureIncompressibleMomentumSPScalarKernel.h"

#include "ADFunctorInterface.h"
#include "ScalarCoupleable.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("ThermalHydraulicsApp", ADCoupledPressureIncompressibleMomentumSPScalarKernel);

InputParameters
ADCoupledPressureIncompressibleMomentumSPScalarKernel::validParams()
{
  InputParameters params = ADIncompressibleMomentumSPBase::validParams();
  params += ADFunctorInterface::validParams();
  params.addClassDescription("Implements a generic momentum solve over a 1D flow path, acting on "
                             "the reference pressure drop.");
  params.addCoupledVar("coupled_mass_flow_rate",
                       {},
                       "coupled_mass_flow_rate. Takes a "
                       "scalar variable name");

  return params;
}

ADCoupledPressureIncompressibleMomentumSPScalarKernel::
    ADCoupledPressureIncompressibleMomentumSPScalarKernel(const InputParameters & parameters)
  : Base(parameters), _mc(ScalarCoupleable::adCoupledScalarValue("coupled_mass_flow_rate"))
{
}
