//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiffusionHDGDirichletBC.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "Function.h"
#include "DiffusionHDGKernel.h"

registerMooseObject("MooseApp", DiffusionHDGDirichletBC);

InputParameters
DiffusionHDGDirichletBC::validParams()
{
  auto params = HDGIntegratedBC::validParams();
  params += DiffusionHDGAssemblyHelper::validParams();
  params.addClassDescription("Weakly imposes Dirichlet boundary conditions for a "
                             "hybridized discretization of a diffusion equation");
  params.addParam<MooseFunctorName>("functor", 0, "The Dirichlet value for the diffusing specie");
  return params;
}

DiffusionHDGDirichletBC::DiffusionHDGDirichletBC(const InputParameters & parameters)
  : HDGIntegratedBC(parameters),
    DiffusionHDGAssemblyHelper(this, this, this, _sys, _aux_sys, _tid),
    _dirichlet_val(getFunctor<Real>("functor"))
{
}

void
DiffusionHDGDirichletBC::onBoundary()
{
  resizeData();

  // For notation, please read "A superconvergent LDG-hybridizable Galerkin method for second-order
  // elliptic problems" by Cockburn

  // qu, u
  vectorDirichletResidual(0,
                          _dirichlet_val,
                          _JxW_face,
                          *_qrule_face,
                          _normals,
                          _current_elem,
                          _current_side,
                          _q_point_face);
  scalarDirichletResidual(_vector_n_dofs,
                          _qu_sol,
                          _u_sol,
                          _dirichlet_val,
                          _JxW_face,
                          *_qrule_face,
                          _normals,
                          _current_elem,
                          _current_side,
                          _q_point_face);
  scalarDirichletJacobian(_vector_n_dofs, 0, _vector_n_dofs, _JxW_face, *_qrule_face, _normals);

  // Set the LMs on these Dirichlet boundary faces to 0
  createIdentityResidual(_lm_phi_face, _lm_u_sol, _lm_n_dofs, 0);
  createIdentityJacobian(_lm_phi_face, _lm_n_dofs, 0);
}
