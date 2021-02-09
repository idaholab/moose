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
  InputParameters params = FVFluxKernel::validParams();
  params.addParam<Real>(NS::porosity, 1, "porosity");
  MooseEnum momentum_component("x=0 y=1 z=2", "x");
  params.addParam<MooseEnum>("momentum_component",
                             momentum_component,
                             "The component of the momentum equation that this BC applies to.");
  return params;
}

CNSFVMomPressure::CNSFVMomPressure(const InputParameters & params)
  : FVFluxKernel(params),
    _eps(getParam<Real>(nms::porosity)),
    _vel_elem(getADMaterialProperty<RealVectorValue>(nms::velocity)),
    _vel_neighbor(getNeighborADMaterialProperty<RealVectorValue>(nms::velocity)),
    _pressure_elem(getADMaterialProperty<Real>(nms::pressure)),
    _pressure_neighbor(getNeighborADMaterialProperty<Real>(nms::pressure)),
    _index(getParam<MooseEnum>("momentum_component"))
{
}

ADReal
CNSFVMomPressure::computeQpResidual()
{
  ADRealVectorValue v;
  ADReal p_interface;
  Moose::FV::interpolate(
      Moose::FV::InterpMethod::Average, v, _vel_elem[_qp], _vel_neighbor[_qp], *_face_info, true);
  Moose::FV::interpolate(Moose::FV::InterpMethod::Average,
                         p_interface,
                         _pressure_elem[_qp],
                         _pressure_neighbor[_qp],
                         v,
                         *_face_info,
                         true);
  return _normal(_index) * p_interface * _eps;
}
