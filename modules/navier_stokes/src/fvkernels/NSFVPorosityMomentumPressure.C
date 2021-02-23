//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVPorosityMomentumPressure.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", NSFVPorosityMomentumPressure);

InputParameters
NSFVPorosityMomentumPressure::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  params.addClassDescription("Introduces the coupled pressure term multiplied by a porosity into "
                             "the Navier-Stokes momentum equation.");
  params.addParam<MaterialPropertyName>(NS::porosity, NS::porosity, "The porosity");
  params.addParam<MaterialPropertyName>(NS::pressure, NS::pressure, "The pressure");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");
  return params;
}

NSFVPorosityMomentumPressure::NSFVPorosityMomentumPressure(const InputParameters & params)
  : FVFluxKernel(params),
    _eps_elem(getMaterialProperty<Real>(NS::porosity)),
    _eps_neighbor(getNeighborMaterialProperty<Real>(NS::porosity)),
    _p_elem(getADMaterialProperty<Real>(NS::pressure)),
    _p_neighbor(getNeighborADMaterialProperty<Real>(NS::pressure)),
    _index(getParam<MooseEnum>("momentum_component"))
{
}

ADReal
NSFVPorosityMomentumPressure::computeQpResidual()
{
  ADReal eps_p_interface;
  Moose::FV::interpolate(Moose::FV::InterpMethod::Average,
                         eps_p_interface,
                         _p_elem[_qp] * _eps_elem[_qp],
                         _p_neighbor[_qp] * _eps_neighbor[_qp],
                         *_face_info,
                         true);

  return _normal(_index) * eps_p_interface;
}
