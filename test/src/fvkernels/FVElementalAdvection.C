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
  params.addParam<MaterialPropertyName>(
      "advected_quantity",
      "An optional material property name specifying the material property to be advected. If this "
      "is not set, then we will use the value of the nonlinear variable");
  params.addParam<MaterialPropertyName>(
      "grad_advected_quantity",
      "An optional material property gradient name specifying the gradient of the material "
      "property to be advected. If this "
      "is not set, then we will use the gradient of the nonlinear variable");
  return params;
}

FVElementalAdvection::FVElementalAdvection(const InputParameters & params)
  : FVElementalKernel(params),
    _velocity(getParam<RealVectorValue>("velocity")),
    _prop(params.isParamSetByUser("advected_quantity")
              ? &getADMaterialProperty<Real>("advected_quantity")
              : nullptr),
    _grad_prop(params.isParamSetByUser("grad_advected_quantity")
                   ? &getADMaterialProperty<RealVectorValue>("grad_advected_quantity")
                   : nullptr)
{
  const bool both_non_null = _prop && _grad_prop;
  const bool both_null = !_prop && !_grad_prop;
  if (!(both_non_null || both_null))
    mooseError("Either both or none of 'advected_quantity' and 'grad_advected_quantity' must be "
               "supplied for FVElementalAdvection object ",
               name());
}

ADReal
FVElementalAdvection::computeQpResidual()
{
  auto resid =
      _velocity * (_grad_prop ? (*_grad_prop)[_qp] : _var.gradient(makeElemArg(_current_elem)));

  if (_subproblem.getCoordSystem(_current_elem->subdomain_id()) == Moose::COORD_RZ)
  {
    const auto rz_radial_coord = _subproblem.getAxisymmetricRadialCoord();
    resid += _velocity(rz_radial_coord) * (_prop ? (*_prop)[_qp] : _u[_qp]) /
             _q_point[_qp](rz_radial_coord);
  }

  return resid;
}

#endif
