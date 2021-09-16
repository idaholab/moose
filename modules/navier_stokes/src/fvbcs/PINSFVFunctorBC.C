//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVFunctorBC.h"
#include "NS.h"
#include "SinglePhaseFluidProperties.h"
#include "Function.h"
#include "MfrPostprocessor.h"

registerMooseObject("NavierStokesApp", PINSFVFunctorBC);

InputParameters
PINSFVFunctorBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params += INSFVFlowBC::validParams();
  params.addClassDescription("Computes the residual of the advective and pressure term (the latter "
                             "when this object is added for the momentum equation) on a boundary.");
  params.addParam<MaterialPropertyName>(NS::density, NS::density, "The density material property");
  MooseEnum eqn("mass momentum");
  params.addRequiredParam<MooseEnum>("eqn", eqn, "The equation you're solving.");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addParam<MooseEnum>("momentum_component",
                             momentum_component,
                             "The component of the momentum equation that this kernel applies to.");
  params.addRequiredCoupledVar(NS::superficial_velocity_x,
                               "The x-component of the superficial velocity");
  params.addCoupledVar(NS::superficial_velocity_y, "The y-component of the superficial velocity");
  params.addCoupledVar(NS::superficial_velocity_z, "The z-component of the superficial velocity");
  params.addRequiredCoupledVar(NS::pressure, "The pressure");
  params.addRequiredCoupledVar(NS::porosity, "The porosity");
  return params;
}

PINSFVFunctorBC::PINSFVFunctorBC(const InputParameters & params)
  : FVFluxBC(params),
    INSFVFlowBC(params),
    _sup_vel_x(getFunctor<MooseVariableFVReal>(NS::superficial_velocity_x, 0)),
    _sup_vel_y(isParamValid(NS::superficial_velocity_y)
                   ? &getFunctor<MooseVariableFVReal>(NS::superficial_velocity_y, 0)
                   : nullptr),
    _sup_vel_z(isParamValid(NS::superficial_velocity_z)
                   ? &getFunctor<MooseVariableFVReal>(NS::superficial_velocity_z, 0)
                   : nullptr),
    _pressure(getFunctor<MooseVariableFVReal>(NS::pressure, 0)),
    _rho(getFunctor<ADReal>(NS::density)),
    _eps(getFunctor<MooseVariableFVReal>(NS::porosity, 0)),
    _eqn(getParam<MooseEnum>("eqn")),
    _index(getParam<MooseEnum>("momentum_component")),
    _cd_limiter()
{
  if ((_eqn == "momentum") && !isParamValid("momentum_component"))
    paramError("eqn",
               "If 'momentum' is specified for 'eqn', then you must provide a parameter "
               "value for 'momentum_component'");
  if ((_eqn != "momentum") && isParamValid("momentum_component"))
    paramError("momentum_component",
               "'momentum_component' should not be specified when the 'eqn' is not 'momentum'");
}

ADReal
PINSFVFunctorBC::computeQpResidual()
{
  const auto ft = _face_info->faceType(_var.name());
  const bool out_of_elem = (ft == FaceInfo::VarFaceNeighbors::ELEM);
  const auto normal = out_of_elem ? _face_info->normal() : Point(-_face_info->normal());

  // No interpolation on a boundary so argument values to fi_elem_is_upwind do not
  // matter
  const auto face = std::make_tuple(_face_info, &_cd_limiter, true, faceArgSubdomains());
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
  else
    mooseError("Unrecognized equation type ", _eqn);
}
