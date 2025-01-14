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

registerMooseObject("MooseApp", DiffusionHDGDirichletBC);

InputParameters
DiffusionHDGDirichletBC::validParams()
{
  auto params = IntegratedBC::validParams();
  params.addClassDescription("Weakly imposes Dirichlet boundary conditions for a "
                             "hybridized discretization of a diffusion equation");
  params.addParam<MooseFunctorName>("functor", 0, "The Dirichlet value for the diffusing specie");
  params += DiffusionHDGKernel::diffusionParams();
  params.renameParam("variable", "u", "The diffusing specie concentration");
  return params;
}

DiffusionHDGDirichletBC::DiffusionHDGDirichletBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    NonADFunctorInterface(this),
    constructDiffusion(),
    _dirichlet_val(getFunctor<Real>("functor")),
    _my_side(libMesh::invalid_uint)
{
  addMooseVariableDependency(&const_cast<MooseVariableFE<Real> &>(_u_var));
  addMooseVariableDependency(&const_cast<MooseVariableFE<RealVectorValue> &>(_grad_u_var));
  addMooseVariableDependency(&const_cast<MooseVariableFE<Real> &>(_u_face_var));
}

void
DiffusionHDGDirichletBC::initialSetup()
{
  // This check must occur after FEProblemBase::init()
  DiffusionHDGKernel::checkCoupling(_fe_problem, _sys.number(), *this);
}

void
DiffusionHDGDirichletBC::computeResidual()
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
  createIdentityResidual(_lm_phi_face, _lm_u_sol, _lm_re);

  addResiduals(_assembly, _vector_re, _qu_dof_indices, _grad_u_var.scalingFactor());
  addResiduals(_assembly, _scalar_re, _u_dof_indices, _u_var.scalingFactor());
  addResiduals(_assembly, _lm_re, _lm_u_dof_indices, _u_face_var.scalingFactor());
}

void
DiffusionHDGDirichletBC::computeJacobian()
{
  _scalar_vector_jac.resize(_u_dof_indices.size(), _qu_dof_indices.size());
  _scalar_scalar_jac.resize(_u_dof_indices.size(), _u_dof_indices.size());
  _lm_lm_jac.resize(_lm_u_dof_indices.size(), _lm_u_dof_indices.size());

  scalarDirichletJacobian(_JxW, *_qrule, _normals, _scalar_vector_jac, _scalar_scalar_jac);
  createIdentityJacobian(_lm_phi_face, _lm_lm_jac);

  addJacobian(
      _assembly, _scalar_vector_jac, _u_dof_indices, _qu_dof_indices, _u_var.scalingFactor());
  addJacobian(
      _assembly, _scalar_scalar_jac, _u_dof_indices, _u_dof_indices, _u_var.scalingFactor());
  addJacobian(
      _assembly, _lm_lm_jac, _lm_u_dof_indices, _lm_u_dof_indices, _u_face_var.scalingFactor());
}

void
DiffusionHDGDirichletBC::vectorDirichletResidual(const Moose::Functor<Real> & dirichlet_value,
                                                 const MooseArray<Real> & JxW_face,
                                                 const QBase & qrule_face,
                                                 const MooseArray<Point> & normals,
                                                 const Elem * const current_elem,
                                                 const unsigned int current_side,
                                                 const MooseArray<Point> & q_point_face,
                                                 DenseVector<Number> & vector_re)
{
  for (const auto qp : make_range(qrule_face.n_points()))
  {
    const auto scalar_value = dirichlet_value(
        Moose::ElemSideQpArg{current_elem, current_side, qp, &qrule_face, q_point_face[qp]},
        determineState());

    // External boundary -> Dirichlet faces -> Vector equation RHS
    for (const auto i : index_range(_qu_dof_indices))
      vector_re(i) -= JxW_face[qp] * (_vector_phi_face[i][qp] * normals[qp]) * scalar_value;
  }
}

void
DiffusionHDGDirichletBC::scalarDirichletResidual(const MooseArray<Gradient> & vector_sol,
                                                 const MooseArray<Number> & scalar_sol,
                                                 const Moose::Functor<Real> & dirichlet_value,
                                                 const MooseArray<Real> & JxW_face,
                                                 const QBase & qrule_face,
                                                 const MooseArray<Point> & normals,
                                                 const Elem * const current_elem,
                                                 const unsigned int current_side,
                                                 const MooseArray<Point> & q_point_face,
                                                 DenseVector<Number> & scalar_re)
{
  for (const auto qp : make_range(qrule_face.n_points()))
  {
    const auto scalar_value = dirichlet_value(
        Moose::ElemSideQpArg{current_elem, current_side, qp, &qrule_face, q_point_face[qp]},
        determineState());

    for (const auto i : index_range(_u_dof_indices))
    {
      // vector
      scalar_re(i) -=
          JxW_face[qp] * _diff[qp] * _scalar_phi_face[i][qp] * (vector_sol[qp] * normals[qp]);

      // scalar from stabilization term
      scalar_re(i) += JxW_face[qp] * _scalar_phi_face[i][qp] * _tau * scalar_sol[qp] * normals[qp] *
                      normals[qp];

      // dirichlet lm from stabilization term
      scalar_re(i) -=
          JxW_face[qp] * _scalar_phi_face[i][qp] * _tau * scalar_value * normals[qp] * normals[qp];
    }
  }
}

void
DiffusionHDGDirichletBC::scalarDirichletJacobian(const MooseArray<Real> & JxW_face,
                                                 const QBase & qrule_face,
                                                 const MooseArray<Point> & normals,
                                                 DenseMatrix<Number> & scalar_vector_jac,
                                                 DenseMatrix<Number> & scalar_scalar_jac)
{
  for (const auto i : index_range(_u_dof_indices))
    for (const auto qp : make_range(qrule_face.n_points()))
    {
      for (const auto j : index_range(_qu_dof_indices))
        scalar_vector_jac(i, j) -= JxW_face[qp] * _diff[qp] * _scalar_phi_face[i][qp] *
                                   (_vector_phi_face[j][qp] * normals[qp]);

      for (const auto j : index_range(_u_dof_indices))
        scalar_scalar_jac(i, j) += JxW_face[qp] * _scalar_phi_face[i][qp] * _tau *
                                   _scalar_phi_face[j][qp] * normals[qp] * normals[qp];
    }
}

void
DiffusionHDGDirichletBC::jacobianSetup()
{
  _my_elem = nullptr;
  _my_side = libMesh::invalid_uint;
}

void
DiffusionHDGDirichletBC::computeOffDiagJacobian(const unsigned int)
{
  if ((_my_elem != _current_elem) || (_my_side != _current_side))
  {
    computeJacobian();
    _my_elem = _current_elem;
    _my_side = _current_side;
  }
}

inline void
DiffusionHDGDirichletBC::createIdentityResidual(const MooseArray<std::vector<Real>> & phi,
                                                const MooseArray<Number> & sol,
                                                DenseVector<Number> & re)
{
  for (const auto qp : make_range(_qrule->n_points()))
    for (const auto i : index_range(phi))
      re(i) -= _JxW[qp] * phi[i][qp] * sol[qp];
}

inline void
DiffusionHDGDirichletBC::createIdentityJacobian(const MooseArray<std::vector<Real>> & phi,
                                                DenseMatrix<Number> & ke)
{
  for (const auto qp : make_range(_qrule->n_points()))
    for (const auto i : index_range(phi))
      for (const auto j : index_range(phi))
        ke(i, j) -= _JxW[qp] * phi[i][qp] * phi[j][qp];
}
