//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NavierStokesHDGOutflowBC.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "Function.h"
#include "NavierStokesHDGKernel.h"

registerMooseObject("NavierStokesApp", NavierStokesHDGOutflowBC);

InputParameters
NavierStokesHDGOutflowBC::validParams()
{
  auto params = HDGIntegratedBC::validParams();
  params += NavierStokesHDGAssemblyHelper::validParams();
  params.addClassDescription("Implements an outflow boundary condition for use with a hybridized "
                             "discretization of the incompressible Navier-Stokes equations");
  return params;
}

NavierStokesHDGOutflowBC::NavierStokesHDGOutflowBC(const InputParameters & parameters)
  : HDGIntegratedBC(parameters),
    NavierStokesHDGAssemblyHelper(this, this, this, _sys, _aux_sys, _mesh, _tid)
{
}

void
NavierStokesHDGOutflowBC::onBoundary()
{
  resizeData();

  // qu, u, lm_u
  vectorFaceResidual(0, _lm_u_sol, _JxW_face, *_qrule_face, _normals);
  vectorFaceJacobian(0, 0, _JxW_face, *_qrule_face, _normals);
  scalarFaceResidual(
      _vector_n_dofs, _qu_sol, _u_sol, _lm_u_sol, 0, _JxW_face, *_qrule_face, _normals);
  scalarFaceJacobian(_vector_n_dofs,
                     0,
                     _vector_n_dofs,
                     0,
                     2 * _lm_n_dofs,
                     0,
                     0,
                     _lm_n_dofs,
                     _JxW_face,
                     *_qrule_face,
                     _normals);
  lmFaceResidual(0, _qu_sol, _u_sol, _lm_u_sol, 0, _JxW_face, *_qrule_face, _normals, _neigh);
  lmFaceJacobian(0,
                 0,
                 _vector_n_dofs,
                 0,
                 2 * _lm_n_dofs,
                 0,
                 0,
                 _lm_n_dofs,
                 _JxW_face,
                 *_qrule_face,
                 _normals,
                 _neigh);

  // qv, v, lm_v
  vectorFaceResidual(_vector_n_dofs + _scalar_n_dofs, _lm_v_sol, _JxW_face, *_qrule_face, _normals);
  vectorFaceJacobian(
      _vector_n_dofs + _scalar_n_dofs, _lm_n_dofs, _JxW_face, *_qrule_face, _normals);
  scalarFaceResidual(2 * _vector_n_dofs + _scalar_n_dofs,
                     _qv_sol,
                     _v_sol,
                     _lm_v_sol,
                     1,
                     _JxW_face,
                     *_qrule_face,
                     _normals);
  scalarFaceJacobian(2 * _vector_n_dofs + _scalar_n_dofs,
                     _vector_n_dofs + _scalar_n_dofs,
                     2 * _vector_n_dofs + _scalar_n_dofs,
                     _lm_n_dofs,
                     2 * _lm_n_dofs,
                     1,
                     0,
                     _lm_n_dofs,
                     _JxW_face,
                     *_qrule_face,
                     _normals);
  lmFaceResidual(
      _lm_n_dofs, _qv_sol, _v_sol, _lm_v_sol, 1, _JxW_face, *_qrule_face, _normals, _neigh);
  lmFaceJacobian(_lm_n_dofs,
                 _vector_n_dofs + _scalar_n_dofs,
                 2 * _vector_n_dofs + _scalar_n_dofs,
                 _lm_n_dofs,
                 2 * _lm_n_dofs,
                 1,
                 0,
                 _lm_n_dofs,
                 _JxW_face,
                 *_qrule_face,
                 _normals,
                 _neigh);

  // p
  pressureFaceResidual(2 * _lm_n_dofs, _JxW_face, *_qrule_face, _normals);
  pressureFaceJacobian(2 * _lm_n_dofs, 0, _lm_n_dofs, _JxW_face, *_qrule_face, _normals);
}
