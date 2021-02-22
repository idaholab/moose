//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVPorosityMatAdvection.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", NSFVPorosityMatAdvection);

InputParameters
NSFVPorosityMatAdvection::validParams()
{
  InputParameters params = FVMatAdvection::validParams();
  params.addClassDescription(
      "Computes the residual of advective term with porosity using finite volume method.");
  return params;
}

NSFVPorosityMatAdvection::NSFVPorosityMatAdvection(const InputParameters & params)
  : FVMatAdvection(params),
    _eps_elem(getMaterialProperty<Real>(NS::porosity)),
    _eps_neighbor(getNeighborMaterialProperty<Real>(NS::porosity))
{
}

ADReal
NSFVPorosityMatAdvection::computeQpResidual()
{
  // Check for large porosity change. If we do have a large change we are going to upwind everything
  // to prevent oscillations due to the discontinuity
  if (std::abs(_eps_elem[_qp] - _eps_neighbor[_qp]) > 0.1)
  {
    ADReal u_interface;

    interpolate(Moose::FV::InterpMethod::Average,
                _v,
                _vel_elem[_qp],
                _vel_neighbor[_qp],
                *_face_info,
                true);
    if (MetaPhysicL::raw_value(_v) * _face_info->normal() > 0)
    {
      _v = _vel_elem[_qp];
      u_interface = _adv_quant_elem[_qp];
    }
    else
    {
      _v = _vel_neighbor[_qp];
      u_interface = _adv_quant_neighbor[_qp];
    }

    return _normal * _v * u_interface;
  }
  else
    return FVMatAdvection::computeQpResidual();
}
