//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PWCNSFVStrongBC.h"
#include "NS.h"
#include "SinglePhaseFluidProperties.h"
#include "Function.h"
#include "MfrPostprocessor.h"

registerMooseObject("NavierStokesApp", PWCNSFVStrongBC);

InputParameters
PWCNSFVStrongBC::validParams()
{
  InputParameters params = PINSFVStrongBC::validParams();
  params.addClassDescription("Computes the residual of advective term using finite volume method.");
  params.addParam<MaterialPropertyName>(NS::cp, NS::cp, "The specific heat capacity");
  params.addRequiredCoupledVar(NS::T_fluid, "The fluid temperature");
  return params;
}

PWCNSFVStrongBC::PWCNSFVStrongBC(const InputParameters & params)
  : PINSFVStrongBC(params),
    _T_fluid(getFunctor<MooseVariableFVReal>(NS::T_fluid, 0)),
    _cp(getFunctorMaterialProperty<ADReal>(NS::cp))
{
}

ADReal
PWCNSFVStrongBC::computeQpResidual()
{
  const auto ft = _face_info->faceType(_var.name());
  const bool out_of_elem = (ft == FaceInfo::VarFaceNeighbors::ELEM);
  const auto normal = out_of_elem ? _face_info->normal() : Point(-_face_info->normal());

  // No interpolation on a boundary so argument values to limiter and fi_elem_is_upwind do not
  // matter
  const auto face = std::make_tuple(_face_info, nullptr, true);
  const VectorValue<ADReal> sup_vel(_sup_vel_x(face),
                                    _sup_vel_y ? (*_sup_vel_y)(face) : ADReal(0),
                                    _sup_vel_z ? (*_sup_vel_z)(face) : ADReal(0));
  const auto rho = _rho(face);

  if (_eqn == "mass")
    return rho * sup_vel * normal;
  else if (_eqn == "momentum")
  {
    const auto eps = _eps(face);
    const auto rhou = sup_vel(_index) / eps * rho;
    return rhou * sup_vel * normal + eps * _pressure(face) * normal(_index);
  }
  else if (_eqn == "energy")
  {
    const auto rho_ht = rho * _cp(face) * _T_fluid(face);
    return rho_ht * sup_vel * normal;
  }
  else
    mooseError("Unrecognized equation type ", _eqn);
}
