//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MDMomentumFreeSlipBC.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "SystemBase.h"
#include "FEProblemBase.h"
#include "libmesh/numeric_vector.h"

registerMooseObject("NavierStokesApp", MDMomentumFreeSlipBC);

InputParameters
MDMomentumFreeSlipBC::validParams()
{
  InputParameters params = NodalNormalBC::validParams();
  params.addRequiredCoupledVar("u", "x-component of velocity");
  params.addCoupledVar("v", "y-component of velocity");
  params.addCoupledVar("w", "z-component of velocity");

  return params;
}

MDMomentumFreeSlipBC::MDMomentumFreeSlipBC(const InputParameters & parameters)
  : NodalNormalBC(parameters),
    _mesh_dimension(_mesh.dimension()),
    _u_vel(coupledValue("u")),
    _v_vel(_mesh_dimension >= 2 ? coupledValue("v") : _zero),
    _w_vel(_mesh_dimension >= 3 ? coupledValue("w") : _zero)
{
}

MDMomentumFreeSlipBC::~MDMomentumFreeSlipBC() {}

bool
MDMomentumFreeSlipBC::shouldApply()
{
  // this prevents zeroing out the row
  return !_fe_problem.currentlyComputingJacobian();
}

Real
MDMomentumFreeSlipBC::computeQpResidual()
{
  for (auto tag : _vector_tags)
    if (_sys.hasVector(tag) && _var.isNodalDefined())
    {
      auto & residual = _sys.getVector(tag);

      if (_mesh_dimension == 1)
      {
        mooseError("MDMomentumFreeSlipBC is not applicable for one-dimensional mesh.");
      }
      else if (_mesh_dimension == 2)
      {
        MooseVariable & u_vel_var = *getVar("u", 0);
        auto && u_vel_dof_idx = u_vel_var.nodalDofIndex();

        MooseVariable & v_vel_var = *getVar("v", 0);
        auto && v_vel_dof_idx = v_vel_var.nodalDofIndex();

        Real vel_dot_n = _normal(0) * _u_vel[0] + _normal(1) * _v_vel[0];
        Real Re_u = residual(u_vel_dof_idx);
        Real Re_v = residual(v_vel_dof_idx);

        // The math follows that of the MomentumFreeSlipBC of the Navier-Stokes module
        Real u_vel_val = (Re_u * _normal(1) * _normal(1) - Re_v * _normal(0) * _normal(1)) +
                         vel_dot_n * _normal(0);
        Real v_vel_val = (-Re_u * _normal(0) * _normal(1) + Re_v * _normal(0) * _normal(0)) +
                         vel_dot_n * _normal(1);

        residual.set(u_vel_dof_idx, u_vel_val);
        residual.set(v_vel_dof_idx, v_vel_val);
      }
      else if (_mesh_dimension == 3)
        mooseError("MDMomentumFreeSlipBC has not been implemented for three-dimensional mesh.");
      else
        mooseError("Mesh dimension not supported.");
    }

  return 0.;
}
