//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiffusionLHDGAssemblyHelper.h"
#include "DiffusionLHDGDirichletBC.h"

registerMooseObject("MooseApp", DiffusionLHDGDirichletBC);

InputParameters
DiffusionLHDGDirichletBC::validParams()
{
  auto params = IntegratedBC::validParams();
  params.addClassDescription("Weakly imposes Dirichlet boundary conditions for a "
                             "hybridized discretization of a diffusion equation");
  params.addRequiredParam<MooseFunctorName>("functor", "The Dirichlet value for the diffusing specie");
  params += DiffusionLHDGAssemblyHelper::validParams();
  params.setDocString("variable", "The diffusing specie concentration");
  return params;
}

DiffusionLHDGDirichletBC::DiffusionLHDGDirichletBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    DiffusionLHDGAssemblyHelper(this, this, this, this, _fe_problem, _sys, _tid),
    _dirichlet_val(getFunctor<Real>("functor")),
    _cached_side(libMesh::invalid_uint)
{
}

void
DiffusionLHDGDirichletBC::initialSetup()
{
  // This check must occur after FEProblemBase::init()
  checkCoupling();
}

void
DiffusionLHDGDirichletBC::computeResidual()
{
  // For notation, please read "A superconvergent LDG-hybridizable Galerkin method for second-order
  // elliptic problems" by Cockburn

  _vector_re.resize(_qu_dof_indices.size());
  _scalar_re.resize(_u_dof_indices.size());
  _lm_re.resize(_lm_u_dof_indices.size());

  // qu, u
  vectorDirichletResidual(
      _dirichlet_val, _JxW, *_qrule, _normals, _current_elem, _current_side, _q_point, _vector_re);
  scalarDirichletResidual(_qu_sol,
                          _u_sol,
                          _dirichlet_val,
                          _JxW,
                          *_qrule,
                          _normals,
                          _current_elem,
                          _current_side,
                          _q_point,
                          _scalar_re);

  // Set the LMs on these Dirichlet boundary faces to 0
  createIdentityResidual(_JxW, *_qrule, _lm_phi_face, _lm_u_sol, _lm_re);

  addResiduals(_assembly, _vector_re, _qu_dof_indices, _grad_u_var.scalingFactor());
  addResiduals(_assembly, _scalar_re, _u_dof_indices, _u_var.scalingFactor());
  addResiduals(_assembly, _lm_re, _lm_u_dof_indices, _u_face_var.scalingFactor());
}

void
DiffusionLHDGDirichletBC::computeJacobian()
{
  _scalar_vector_jac.resize(_u_dof_indices.size(), _qu_dof_indices.size());
  _scalar_scalar_jac.resize(_u_dof_indices.size(), _u_dof_indices.size());
  _lm_lm_jac.resize(_lm_u_dof_indices.size(), _lm_u_dof_indices.size());

  scalarDirichletJacobian(_JxW, *_qrule, _normals, _scalar_vector_jac, _scalar_scalar_jac);
  createIdentityJacobian(_JxW, *_qrule, _lm_phi_face, _lm_lm_jac);

  addJacobian(
      _assembly, _scalar_vector_jac, _u_dof_indices, _qu_dof_indices, _u_var.scalingFactor());
  addJacobian(
      _assembly, _scalar_scalar_jac, _u_dof_indices, _u_dof_indices, _u_var.scalingFactor());
  addJacobian(
      _assembly, _lm_lm_jac, _lm_u_dof_indices, _lm_u_dof_indices, _u_face_var.scalingFactor());
}

void
DiffusionLHDGDirichletBC::jacobianSetup()
{
  _cached_elem = nullptr;
  _cached_side = libMesh::invalid_uint;
}

void
DiffusionLHDGDirichletBC::computeOffDiagJacobian(const unsigned int)
{
  if ((_cached_elem != _current_elem) || (_cached_side != _current_side))
  {
    computeJacobian();
    _cached_elem = _current_elem;
    _cached_side = _current_side;
  }
}
