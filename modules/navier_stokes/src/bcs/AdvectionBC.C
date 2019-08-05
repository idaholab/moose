//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdvectionBC.h"
#include "MooseMesh.h"

registerMooseObject("NavierStokesApp", AdvectionBC);

template <>
InputParameters
validParams<AdvectionBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addClassDescription("Boundary conditions for outflow/outflow of advected quantities:"
                             "\n phi * velocity * normal, where phi is the advected quantitiy");
  params.addParam<bool>(
      "outflow", true, "Determines if this BC is applied on inflow or outflow BCs");
  params.addRequiredCoupledVar("velocity_vector",
                               "The components of the velocity vector up to problem dimension");
  return params;
}

AdvectionBC::AdvectionBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _dim(_mesh.dimension()),
    _outflow(getParam<bool>("outflow")),
    _coupled_components(coupledComponents("velocity_vector"))
{
  if (_dim > _coupled_components)
    paramError(
        "velocity_vector",
        "Number of components of velocity_vector must be at least equal to the mesh dimension");
  if (_coupled_components > 3)
    paramError("velocity_vector",
               "You cannot supply more than 3 components for the velocity vector");

  _velocity.resize(_coupled_components);
  for (unsigned int j = 0; j < _coupled_components; ++j)
    _velocity[j] = &coupledValue("velocity_vector", j);
}

Real
AdvectionBC::computeQpResidual()
{
  RealVectorValue vel;
  for (unsigned int j = 0; j < _coupled_components; ++j)
    vel(j) = (*_velocity[j])[_qp];
  if ((vel * _normals[_qp] > 0) == _outflow)
    return _test[_i][_qp] * _u[_qp] * vel * _normals[_qp];
  return 0;
}

Real
AdvectionBC::computeQpJacobian()
{
  RealVectorValue vel;
  for (unsigned int j = 0; j < _coupled_components; ++j)
    vel(j) = (*_velocity[j])[_qp];
  if ((vel * _normals[_qp] > 0) == _outflow)
    return _test[_i][_qp] * _phi[_j][_qp] * vel * _normals[_qp];
  return 0;
}
