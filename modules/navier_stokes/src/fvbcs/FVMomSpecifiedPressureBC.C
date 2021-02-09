//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "FVMomSpecifiedPressureBC.h"
#include "NS.h"

namespace nms = NS;

// Full specialization of the validParams function for this object
registerADMooseObject("NavierStokesApp", CNSFVMomSpecifiedPressureBC);

InputParameters
CNSFVMomSpecifiedPressureBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params.addParam<Real>(nms::porosity, 1, "porosity");
  params.addRequiredParam<PostprocessorName>("specified_pressure", "Specified pressure.");
  MooseEnum momentum_component("x=0 y=1 z=2", "x");
  params.addParam<MooseEnum>("momentum_component",
                             momentum_component,
                             "The component of the momentum equation that this BC applies to.");
  return params;
}

CNSFVMomSpecifiedPressureBC::CNSFVMomSpecifiedPressureBC(const InputParameters & parameters)
  : FVFluxBC(parameters),
    _eps(getParam<Real>(nms::porosity)),
    _pressure(this->getPostprocessorValue("specified_pressure")),
    _index(getParam<MooseEnum>("momentum_component"))
{
}

ADReal
CNSFVMomSpecifiedPressureBC::computeQpResidual()
{
  return _normal(_index) * _pressure * _eps;
}
