//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PNSFVUpwindAdvectorAndAdvected.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", PNSFVUpwindAdvectorAndAdvected);

InputParameters
PNSFVUpwindAdvectorAndAdvected::validParams()
{
  InputParameters params = FVMatAdvection::validParams();
  params.addClassDescription(
      "Computes the residual of advective term with porosity using finite volume method.");
  return params;
}

PNSFVUpwindAdvectorAndAdvected::PNSFVUpwindAdvectorAndAdvected(const InputParameters & params)
  : FVMatAdvection(params),
    _eps_elem(getMaterialProperty<Real>(NS::porosity)),
    _eps_neighbor(getNeighborMaterialProperty<Real>(NS::porosity))
{
}

ADReal
PNSFVUpwindAdvectorAndAdvected::computeQpResidual()
{
  ADReal u_interface;
  Real elem_coeff, neighbor_coeff;

  interpolate(
      Moose::FV::InterpMethod::Average, _v, _vel_elem[_qp], _vel_neighbor[_qp], *_face_info, true);
  if (MetaPhysicL::raw_value(_v) * _face_info->normal() > 0)
  {
    elem_coeff = 1;
    neighbor_coeff = 0;
  }
  else
  {
    elem_coeff = 0;
    neighbor_coeff = 1;
  }
  _v = _vel_elem[_qp] * elem_coeff + _vel_neighbor[_qp] * neighbor_coeff;
  u_interface = _adv_quant_elem[_qp] * elem_coeff + _adv_quant_neighbor[_qp] * neighbor_coeff;

  return _normal * _v * u_interface;
}
