//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADIncompressibleMomentumSPScalarKernel.h"

#include "ADFunctorInterface.h"
#include "ScalarCoupleable.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("ThermalHydraulicsApp", ADIncompressibleMomentumSPScalarKernel);

InputParameters
ADIncompressibleMomentumSPScalarKernel::validParams()
{
  InputParameters params = ADIncompressibleMomentumSPBase::validParams();
  params += ADFunctorInterface::validParams();
  params.addClassDescription(
      "Implements a generic momentum solve over a 1D flow path, acting on the mass flow rate.");
  params.addCoupledVar("reference_pressure_drop",
                       {},
                       "Reference system pressure drop from inlet to outlet. Takes a "
                       "scalar variable name");
  return params;
}

ADIncompressibleMomentumSPScalarKernel::ADIncompressibleMomentumSPScalarKernel(
    const InputParameters & parameters)
  : Base(parameters), _dPc(ScalarCoupleable::adCoupledScalarValue("reference_pressure_drop"))
{
}
