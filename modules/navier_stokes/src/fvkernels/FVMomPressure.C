//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVMomPressure.h"
#include "NS.h"

namespace nms = NS;

// Full specialization of the validParams function for this object
registerADMooseObject("NavierStokesApp", CNSFVMomPressure);

InputParameters
CNSFVMomPressure::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addParam<Real>(NS::porosity, 1, "porosity");
  MooseEnum momentum_component("x=0 y=1 z=2", "x");
  params.addParam<MooseEnum>("momentum_component",
                             momentum_component,
                             "The component of the momentum equation that this BC applies to.");
  return params;
}

CNSFVMomPressure::CNSFVMomPressure(const InputParameters & params)
  : FVElementalKernel(params),
    _eps(getParam<Real>(nms::porosity)),
    _grad_pressure(getADMaterialProperty<RealVectorValue>(nms::grad(nms::pressure))),
    _index(getParam<MooseEnum>("momentum_component"))
{
}

ADReal
CNSFVMomPressure::computeQpResidual()
{
  return _grad_pressure[_qp](_index);
}
