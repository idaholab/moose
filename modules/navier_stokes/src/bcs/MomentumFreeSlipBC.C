//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MomentumFreeSlipBC.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "SystemBase.h"
#include "FEProblemBase.h"
#include "libmesh/numeric_vector.h"

registerMooseObject("NavierStokesApp", MomentumFreeSlipBC);

InputParameters
MomentumFreeSlipBC::validParams()
{
  InputParameters params = NodalNormalBC::validParams();
  params.addRequiredCoupledVar("rho_u", "x-component of velocity");
  params.addCoupledVar("rho_v", "y-component of velocity");
  params.addCoupledVar("rho_w", "z-component of velocity");
  params.addClassDescription("Implements free slip boundary conditions for the Navier Stokes"
                             "momentum equation.");
  return params;
}

MomentumFreeSlipBC::MomentumFreeSlipBC(const InputParameters & parameters)
  : NodalNormalBC(parameters),
    _mesh_dimension(_mesh.dimension()),
    _rho_u(coupledValue("rho_u")),
    _rho_v(_mesh_dimension >= 2 ? coupledValue("rho_v") : _zero),
    _rho_w(_mesh_dimension >= 3 ? coupledValue("rho_w") : _zero)
{
}

MomentumFreeSlipBC::~MomentumFreeSlipBC() {}

bool
MomentumFreeSlipBC::shouldApply()
{
  // this prevents zeroing out the row
  return !_fe_problem.currentlyComputingJacobian();
}

Real
MomentumFreeSlipBC::computeQpResidual()
{
  for (auto tag : _vector_tags)
    if (_sys.hasVector(tag) && _var.isNodalDefined())
    {
      auto & residual = _sys.getVector(tag);

      if (_mesh_dimension == 1)
      {
        mooseError(name(), ": Not implemented yet");
      }
      else if (_mesh_dimension == 2)
      {
        MooseVariable & rho_u_var = *getVar("rho_u", 0);
        auto && rho_u_dof_idx = rho_u_var.nodalDofIndex();

        MooseVariable & rho_v_var = *getVar("rho_v", 0);
        auto && rho_v_dof_idx = rho_v_var.nodalDofIndex();

        Real rho_un = _normal(0) * _rho_u[0] + _normal(1) * _rho_v[0];
        Real Re_u = residual(rho_u_dof_idx);
        Real Re_v = residual(rho_v_dof_idx);

        // We obtain these contributions in 3 steps:
        // 1.) Tranform the momentum residuals into (tangential, normal)
        //     components by left-multiplying the residual by:
        //     R = [tx ty] = [-ny nx]
        //         [nx ny]   [ nx ny]
        // 2.) Impose the no-normal-flow BC, rho_un = 0, in the normal momentum component's
        // residual.
        // 3.) Transform back to (x,y) momentum components by left-multiplying the residual by
        // R^{-1} = R^T.
        Real rho_u_val =
            (Re_u * _normal(1) * _normal(1) - Re_v * _normal(0) * _normal(1)) + rho_un * _normal(0);
        Real rho_v_val = (-Re_u * _normal(0) * _normal(1) + Re_v * _normal(0) * _normal(0)) +
                         rho_un * _normal(1);

        // NOTE: we have to handle all components at the same time, otherwise we'd work with the
        // modified residual and we do not want that
        residual.set(rho_u_dof_idx, rho_u_val);
        residual.set(rho_v_dof_idx, rho_v_val);
      }
      else if (_mesh_dimension == 3)
      {
        mooseError(name(), ": Not implemented yet");
      }
    }

  // The return value is not used for anything, so we simply return 0.
  return 0.;
}
