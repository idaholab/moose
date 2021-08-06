//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMaterial.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", INSFVMaterial);

InputParameters
INSFVMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addClassDescription("This is the material class used to compute advected quantities for "
                             "the finite-volume implementation of the Navier-Stokes equations.");
  params.addRequiredCoupledVar("u", "The x-velocity");
  params.addCoupledVar("v", "y-velocity"); // only required in 2D and 3D
  params.addCoupledVar("w", "z-velocity"); // only required in 3D
  params.addRequiredParam<MaterialPropertyName>("rho", "The value for the density");
  params.addRequiredCoupledVar(NS::pressure, "The pressure variable.");
  params.addCoupledVar("temperature", "the temperature");
  params.addParam<MaterialPropertyName>("cp_name", "cp", "the name of the specific heat capacity");
  return params;
}

INSFVMaterial::INSFVMaterial(const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _u_vel(*getVarHelper<MooseVariableFVReal>("u", 0)),
    _v_vel(isCoupled("v") ? getVarHelper<MooseVariableFVReal>("v", 0) : nullptr),
    _w_vel(isCoupled("w") ? getVarHelper<MooseVariableFVReal>("w", 0) : nullptr),
    _p_var(*getVarHelper<MooseVariableFVReal>(NS::pressure, 0)),
    _rho(getFunctor<ADReal>("rho")),
    _has_temperature(isParamValid("temperature")),
    _temperature(_has_temperature ? getVarHelper<MooseVariableFVReal>("temperature", 0) : nullptr),
    _cp(_has_temperature ? &getFunctor<ADReal>("cp_name") : nullptr)
{
  if (_mesh.dimension() >= 2 && !_v_vel)
    mooseError(
        "If the mesh dimension is 2 or greater, then a 'v' variable parameter must be supplied");
  if (_mesh.dimension() >= 3 && !_w_vel)
    mooseError("If the mesh dimension is 3, then a 'w' variable parameter must be supplied");

  addFunctorProperty<ADRealVectorValue>(NS::velocity,
                                        [this](const auto & r, const auto & t) -> ADRealVectorValue
                                        {
                                          ADRealVectorValue velocity(_u_vel(r, t));
                                          if (_mesh.dimension() >= 2)
                                            velocity(1) = (*_v_vel)(r, t);
                                          if (_mesh.dimension() >= 3)
                                            velocity(2) = (*_w_vel)(r, t);
                                          return velocity;
                                        });
  addFunctorProperty<ADReal>(NS::momentum_x,
                             [this](const auto & r, const auto & t) -> ADReal
                             { return _rho(r, t) * _u_vel(r, t); });
  addFunctorProperty<ADReal>(NS::momentum_y,
                             [this](const auto & r, const auto & t) -> ADReal
                             {
                               if (_mesh.dimension() >= 2)
                                 return _rho(r, t) * (*_v_vel)(r, t);
                               else
                                 return 0;
                             });
  addFunctorProperty<ADReal>(NS::momentum_z,
                             [this](const auto & r, const auto & t) -> ADReal
                             {
                               if (_mesh.dimension() >= 3)
                                 return _rho(r, t) * (*_w_vel)(r, t);
                               else
                                 return 0;
                             });
  if (_has_temperature)
    addFunctorProperty<ADReal>("rho_cp_temp",
                               [this](const auto & r, const auto & t) -> ADReal
                               { return _rho(r, t) * (*_cp)(r, t) * (*_temperature)(r, t); });
}
