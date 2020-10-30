//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVElementalAdvection.h"

#ifdef MOOSE_GLOBAL_AD_INDEXING

registerMooseObject("MooseTestApp", FVElementalAdvection);

InputParameters
FVElementalAdvection::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addRequiredParam<RealVectorValue>("velocity", "Constant advection velocity");
  return params;
}

FVElementalAdvection::FVElementalAdvection(const InputParameters & params)
  : FVElementalKernel(params), _velocity(getParam<RealVectorValue>("velocity"))
{
}

ADReal
FVElementalAdvection::computeQpResidual()
{
  auto resid = _velocity * _var.adGradSln(_current_elem);

  if (_subproblem.getCoordSystem(_current_elem->subdomain_id()) == Moose::COORD_RZ)
  {
    const auto rz_radial_coord = _subproblem.getAxisymmetricRadialCoord();
    resid += _velocity(rz_radial_coord) * _u[_qp] / _q_point[_qp](rz_radial_coord);
  }

  return resid;
}

#endif
