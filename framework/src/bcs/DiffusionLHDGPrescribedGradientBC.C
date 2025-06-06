//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiffusionLHDGAssemblyHelper.h"
#include "DiffusionLHDGPrescribedGradientBC.h"

registerMooseObject("MooseApp", DiffusionLHDGPrescribedGradientBC);

InputParameters
DiffusionLHDGPrescribedGradientBC::validParams()
{
  auto params = IntegratedBC::validParams();
  params += DiffusionLHDGAssemblyHelper::validParams();
  params.renameParam("variable", "u", "The diffusing specie concentration");
  params.addClassDescription("Implements a flux boundary condition for use with a hybridized "
                             "discretization of the diffusion equation");
  params.addParam<MooseFunctorName>(
      "normal_gradient", 0, "The prescribed value of the gradient dotted with the normal");
  return params;
}

DiffusionLHDGPrescribedGradientBC::DiffusionLHDGPrescribedGradientBC(
    const InputParameters & parameters)
  : IntegratedBC(parameters),
    DiffusionLHDGAssemblyHelper(this, this, this, this, _fe_problem, _sys, _tid),
    _normal_gradient(getFunctor<Real>("normal_gradient")),
    _cached_side(libMesh::invalid_uint)
{
}

void
DiffusionLHDGPrescribedGradientBC::initialSetup()
{
  checkCoupling();
}

void
DiffusionLHDGPrescribedGradientBC::computeResidual()
{
  _vector_re.resize(_qu_dof_indices.size());
  _scalar_re.resize(_u_dof_indices.size());
  _lm_re.resize(_lm_u_dof_indices.size());

  // For notation, please read "A superconvergent LDG-hybridizable Galerkin method for second-order
  // elliptic problems" by Cockburn

  // qu, u, lm_u
  vectorFaceResidual(_lm_u_sol, _JxW, *_qrule, _normals, _vector_re);
  scalarFaceResidual(_qu_sol, _u_sol, _lm_u_sol, _JxW, *_qrule, _normals, _scalar_re);
  lmFaceResidual(_qu_sol, _u_sol, _lm_u_sol, _JxW, *_qrule, _normals, _lm_re);

  for (const auto qp : make_range(_qrule->n_points()))
    for (const auto i : index_range(_lm_re))
      // prescribed normal gradient
      _lm_re(i) += _JxW[qp] * _diff[qp] * _lm_phi_face[i][qp] *
                   _normal_gradient(
                       Moose::ElemSideQpArg{_current_elem, _current_side, qp, _qrule, _q_point[qp]},
                       determineState());

  addResiduals(_assembly, _vector_re, _qu_dof_indices, _grad_u_var.scalingFactor());
  addResiduals(_assembly, _scalar_re, _u_dof_indices, _u_var.scalingFactor());
  addResiduals(_assembly, _lm_re, _lm_u_dof_indices, _u_face_var.scalingFactor());
}

void
DiffusionLHDGPrescribedGradientBC::computeJacobian()
{
  _vector_vector_jac.resize(_qu_dof_indices.size(), _qu_dof_indices.size());
  _vector_scalar_jac.resize(_qu_dof_indices.size(), _u_dof_indices.size());
  _scalar_vector_jac.resize(_u_dof_indices.size(), _qu_dof_indices.size());
  _scalar_scalar_jac.resize(_u_dof_indices.size(), _u_dof_indices.size());
  _scalar_lm_jac.resize(_u_dof_indices.size(), _lm_u_dof_indices.size());
  _lm_scalar_jac.resize(_lm_u_dof_indices.size(), _u_dof_indices.size());
  _lm_lm_jac.resize(_lm_u_dof_indices.size(), _lm_u_dof_indices.size());
  _vector_lm_jac.resize(_qu_dof_indices.size(), _lm_u_dof_indices.size());
  _lm_vector_jac.resize(_lm_u_dof_indices.size(), _qu_dof_indices.size());

  // qu, u, lm_u
  vectorFaceJacobian(_JxW, *_qrule, _normals, _vector_lm_jac);
  scalarFaceJacobian(
      _JxW, *_qrule, _normals, _scalar_vector_jac, _scalar_scalar_jac, _scalar_lm_jac);
  lmFaceJacobian(_JxW, *_qrule, _normals, _lm_vector_jac, _lm_scalar_jac, _lm_lm_jac);

  addJacobian(
      _assembly, _vector_vector_jac, _qu_dof_indices, _qu_dof_indices, _grad_u_var.scalingFactor());
  addJacobian(
      _assembly, _vector_scalar_jac, _qu_dof_indices, _u_dof_indices, _grad_u_var.scalingFactor());
  addJacobian(
      _assembly, _scalar_vector_jac, _u_dof_indices, _qu_dof_indices, _u_var.scalingFactor());
  addJacobian(
      _assembly, _scalar_scalar_jac, _u_dof_indices, _u_dof_indices, _u_var.scalingFactor());
  addJacobian(_assembly, _scalar_lm_jac, _u_dof_indices, _lm_u_dof_indices, _u_var.scalingFactor());
  addJacobian(
      _assembly, _lm_scalar_jac, _lm_u_dof_indices, _u_dof_indices, _u_face_var.scalingFactor());
  addJacobian(
      _assembly, _lm_lm_jac, _lm_u_dof_indices, _lm_u_dof_indices, _u_face_var.scalingFactor());
  addJacobian(
      _assembly, _vector_lm_jac, _qu_dof_indices, _lm_u_dof_indices, _grad_u_var.scalingFactor());
  addJacobian(
      _assembly, _lm_vector_jac, _lm_u_dof_indices, _qu_dof_indices, _u_face_var.scalingFactor());
}

void
DiffusionLHDGPrescribedGradientBC::jacobianSetup()
{
  _cached_elem = nullptr;
  _cached_side = libMesh::invalid_uint;
}

void
DiffusionLHDGPrescribedGradientBC::computeOffDiagJacobian(const unsigned int)
{
  if ((_cached_elem != _current_elem) || (_cached_side != _current_side))
  {
    computeJacobian();
    _cached_elem = _current_elem;
    _cached_side = _current_side;
  }
}
