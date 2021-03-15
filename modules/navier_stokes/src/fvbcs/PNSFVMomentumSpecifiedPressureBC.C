//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PNSFVMomentumSpecifiedPressureBC.h"
#include "NS.h"
#include "Function.h"

registerMooseObject("NavierStokesApp", PNSFVMomentumSpecifiedPressureBC);

InputParameters
PNSFVMomentumSpecifiedPressureBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params.addClassDescription("Specifies a pressure at a boundary for the momentum equations.");
  params.addRequiredParam<FunctionName>(NS::pressure, "pressure specified as a function");
  params.addParam<MaterialPropertyName>(NS::porosity, NS::porosity, "The porosity");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");
  return params;
}

PNSFVMomentumSpecifiedPressureBC::PNSFVMomentumSpecifiedPressureBC(const InputParameters & params)
  : FVFluxBC(params),
    _pressure(getFunction(NS::pressure)),
    _eps_elem(getMaterialProperty<Real>(NS::porosity)),
    _eps_neighbor(getNeighborMaterialProperty<Real>(NS::porosity)),
    _index(getParam<MooseEnum>("momentum_component"))
{
}

ADReal
PNSFVMomentumSpecifiedPressureBC::computeQpResidual()
{
  mooseAssert(_eps_elem[_qp] == _eps_neighbor[_qp],
              "It's not good if a ghost cell porosity has a different porosity than the boundary "
              "cell porosity");

  return _normal(_index) * _eps_elem[_qp] * _pressure.value(_t, _face_info->faceCentroid());
}
