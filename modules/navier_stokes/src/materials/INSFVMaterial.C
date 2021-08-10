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
  InputParameters params = Material::validParams();
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
  : Material(parameters),
    _u_vel(*getVarHelper<MooseVariableFVReal>("u", 0)),
    _v_vel(isCoupled("v") ? getVarHelper<MooseVariableFVReal>("v", 0) : nullptr),
    _w_vel(isCoupled("w") ? getVarHelper<MooseVariableFVReal>("w", 0) : nullptr),
    _p_var(*getVarHelper<MooseVariableFVReal>(NS::pressure, 0)),
    _velocity(declareFunctorProperty<ADRealVectorValue>(NS::velocity)),
    _rho_u(declareFunctorProperty<ADReal>(NS::momentum_x)),
    _rho_v(declareFunctorProperty<ADReal>(NS::momentum_y)),
    _rho_w(declareFunctorProperty<ADReal>(NS::momentum_z)),
    _p(declareFunctorProperty<ADReal>(NS::pressure)),
    _rho(getFunctorMaterialProperty<ADReal>("rho")),
    _has_temperature(isParamValid("temperature")),
    _temperature(_has_temperature ? getVarHelper<MooseVariableFVReal>("temperature", 0) : nullptr),
    _cp(_has_temperature ? &getFunctorMaterialProperty<ADReal>("cp_name") : nullptr),
    _rho_cp_temp(_has_temperature ? &declareFunctorProperty<ADReal>("rho_cp_temp") : nullptr)
{
  if (_mesh.dimension() >= 2 && !_v_vel)
    mooseError(
        "If the mesh dimension is 2 or greater, then a 'v' variable parameter must be supplied");
  if (_mesh.dimension() >= 3 && !_w_vel)
    mooseError("If the mesh dimension is 3, then a 'w' variable parameter must be supplied");

  _p.setFunctor(
      _mesh, blockIDs(), [this](auto & geom_quantity) -> ADReal { return _p_var(geom_quantity); });
  _velocity.setFunctor(_mesh, blockIDs(), [this](auto & geom_quantity) -> ADRealVectorValue {
    ADRealVectorValue velocity(_u_vel(geom_quantity));
    if (_mesh.dimension() >= 2)
      velocity(1) = (*_v_vel)(geom_quantity);
    if (_mesh.dimension() >= 3)
      velocity(2) = (*_w_vel)(geom_quantity);
    return velocity;
  });
  _rho_u.setFunctor(_mesh, blockIDs(), [this](auto & geom_quantity) -> ADReal {
    return _rho(geom_quantity) * _u_vel(geom_quantity);
  });
  _rho_v.setFunctor(_mesh, blockIDs(), [this](auto & geom_quantity) -> ADReal {
    if (_mesh.dimension() >= 2)
      return _rho(geom_quantity) * (*_v_vel)(geom_quantity);
    else
      return 0;
  });
  _rho_w.setFunctor(_mesh, blockIDs(), [this](auto & geom_quantity) -> ADReal {
    if (_mesh.dimension() >= 3)
      return _rho(geom_quantity) * (*_w_vel)(geom_quantity);
    else
      return 0;
  });
  if (_has_temperature)
    _rho_cp_temp->setFunctor(_mesh, blockIDs(), [this](auto & geom_quantity) -> ADReal {
      return _rho(geom_quantity) * (*_cp)(geom_quantity) * (*_temperature)(geom_quantity);
    });
}
