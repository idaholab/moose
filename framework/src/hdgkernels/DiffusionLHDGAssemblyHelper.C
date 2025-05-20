//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ActionWarehouse.h"
#include "DiffusionLHDGAssemblyHelper.h"
#include "MooseVariableDependencyInterface.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "Function.h"
#include "SystemBase.h"
#include "MooseMesh.h"
#include "MooseObject.h"
#include "MaterialPropertyInterface.h"
#include "NonlinearThread.h"

using namespace libMesh;

InputParameters
DiffusionLHDGAssemblyHelper::validParams()
{
  auto params = emptyInputParameters();
  params.addRequiredParam<NonlinearVariableName>(
      "gradient_variable", "The gradient of the diffusing specie concentration");
  params.addRequiredParam<NonlinearVariableName>(
      "face_variable", "The concentration of the diffusing specie on faces");
  params.addRequiredParam<MaterialPropertyName>("diffusivity", "The diffusivity");
  params.addParam<Real>("tau",
                        1,
                        "The stabilization coefficient required for discontinuous Galerkin "
                        "schemes. This may be set to 0 for a mixed method with Raviart-Thomas.");
  return params;
}

DiffusionLHDGAssemblyHelper::DiffusionLHDGAssemblyHelper(
    const MooseObject * const moose_obj,
    MaterialPropertyInterface * const mpi,
    MooseVariableDependencyInterface * const mvdi,
    const TransientInterface * const ti,
    const FEProblemBase & fe_problem,
    SystemBase & sys,
    const THREAD_ID tid)
  : NonADFunctorInterface(moose_obj),
    _u_var(sys.getFieldVariable<Real>(tid, moose_obj->getParam<NonlinearVariableName>("variable"))),
    _grad_u_var(sys.getFieldVariable<RealVectorValue>(
        tid, moose_obj->getParam<NonlinearVariableName>("gradient_variable"))),
    _u_face_var(sys.getFieldVariable<Real>(
        tid, moose_obj->getParam<NonlinearVariableName>("face_variable"))),
    _qu_dof_indices(_grad_u_var.dofIndices()),
    _u_dof_indices(_u_var.dofIndices()),
    _lm_u_dof_indices(_u_face_var.dofIndices()),
    _qu_sol(_grad_u_var.sln()),
    _u_sol(_u_var.sln()),
    _lm_u_sol(_u_face_var.sln()),
    _vector_phi(_grad_u_var.phi()),
    _scalar_phi(_u_var.phi()),
    _grad_scalar_phi(_u_var.gradPhi()),
    _div_vector_phi(_grad_u_var.divPhi()),
    _vector_phi_face(_grad_u_var.phiFace()),
    _scalar_phi_face(_u_var.phiFace()),
    _lm_phi_face(_u_face_var.phiFace()),
    _diff(mpi->getMaterialProperty<Real>("diffusivity")),
    _ti(*ti),
    _tau(moose_obj->getParam<Real>("tau")),
    _my_elem(nullptr),
    _moose_obj(*moose_obj),
    _dhah_fe_problem(fe_problem),
    _dhah_sys(sys)
{
  mvdi->addMooseVariableDependency(&const_cast<MooseVariableFE<Real> &>(_u_var));
  mvdi->addMooseVariableDependency(&const_cast<MooseVariableFE<RealVectorValue> &>(_grad_u_var));
  mvdi->addMooseVariableDependency(&const_cast<MooseVariableFE<Real> &>(_u_face_var));
}

void
DiffusionLHDGAssemblyHelper::checkCoupling()
{
  // This check must occur after FEProblemBase::init()
  if (_dhah_fe_problem.coupling() == Moose::COUPLING_FULL)
    return;
  else if (_dhah_fe_problem.coupling() == Moose::COUPLING_CUSTOM)
  {
    const auto * const cm = _dhah_fe_problem.couplingMatrix(_dhah_sys.number());
    for (const auto i : make_range(cm->size()))
      for (const auto j : make_range(cm->size()))
        if ((*cm)(i, j) != true)
          goto error;

    return;
  }

error:
  _moose_obj.mooseError(
      "This class encodes the full Jacobian regardless of user input file specification, "
      "so please request full coupling in your Preconditioning block for consistency");
}

void
DiffusionLHDGAssemblyHelper::vectorVolumeResidual(const MooseArray<Gradient> & vector_sol,
                                                  const MooseArray<Number> & scalar_sol,
                                                  const MooseArray<Real> & JxW,
                                                  const QBase & qrule,
                                                  DenseVector<Number> & vector_re)
{
  for (const auto i : index_range(vector_re))
    for (const auto qp : make_range(qrule.n_points()))
    {
      // Vector equation dependence on vector dofs
      vector_re(i) += JxW[qp] * (_vector_phi[i][qp] * vector_sol[qp]);

      // Vector equation dependence on scalar dofs
      vector_re(i) += JxW[qp] * (_div_vector_phi[i][qp] * scalar_sol[qp]);
    }
}

void
DiffusionLHDGAssemblyHelper::vectorVolumeJacobian(const MooseArray<Real> & JxW,
                                                  const QBase & qrule,
                                                  DenseMatrix<Number> & vector_vector_jac,
                                                  DenseMatrix<Number> & vector_scalar_jac)
{
  for (const auto i : make_range(vector_vector_jac.m()))
    for (const auto qp : make_range(qrule.n_points()))
    {
      // Vector equation dependence on vector dofs
      for (const auto j : make_range(vector_vector_jac.n()))
        vector_vector_jac(i, j) += JxW[qp] * (_vector_phi[i][qp] * _vector_phi[j][qp]);

      // Vector equation dependence on scalar dofs
      for (const auto j : make_range(vector_scalar_jac.n()))
        vector_scalar_jac(i, j) += JxW[qp] * (_div_vector_phi[i][qp] * _scalar_phi[j][qp]);
    }
}

void
DiffusionLHDGAssemblyHelper::scalarVolumeResidual(const MooseArray<Gradient> & vector_field,
                                                  const Moose::Functor<Real> & source,
                                                  const MooseArray<Real> & JxW,
                                                  const QBase & qrule,
                                                  const Elem * const current_elem,
                                                  const MooseArray<Point> & q_point,
                                                  DenseVector<Number> & scalar_re)
{
  for (const auto qp : make_range(qrule.n_points()))
  {
    // Evaluate source
    const auto f =
        source(Moose::ElemQpArg{current_elem, qp, &qrule, q_point[qp]}, _ti.determineState());

    for (const auto i : index_range(scalar_re))
    {
      scalar_re(i) += JxW[qp] * (_grad_scalar_phi[i][qp] * _diff[qp] * vector_field[qp]);

      // Scalar equation RHS
      scalar_re(i) -= JxW[qp] * _scalar_phi[i][qp] * f;
    }
  }
}

void
DiffusionLHDGAssemblyHelper::scalarVolumeJacobian(const MooseArray<Real> & JxW,
                                                  const QBase & qrule,
                                                  DenseMatrix<Number> & scalar_vector_jac)
{
  for (const auto i : make_range(scalar_vector_jac.m()))
    // Scalar equation dependence on vector dofs
    for (const auto j : make_range(scalar_vector_jac.n()))
      for (const auto qp : make_range(qrule.n_points()))
        scalar_vector_jac(i, j) +=
            JxW[qp] * _diff[qp] * (_grad_scalar_phi[i][qp] * _vector_phi[j][qp]);
}

void
DiffusionLHDGAssemblyHelper::vectorFaceResidual(const MooseArray<Number> & lm_sol,
                                                const MooseArray<Real> & JxW_face,
                                                const QBase & qrule_face,
                                                const MooseArray<Point> & normals,
                                                DenseVector<Number> & vector_re)
{
  // Vector equation dependence on LM dofs
  for (const auto i : index_range(vector_re))
    for (const auto qp : make_range(qrule_face.n_points()))
      vector_re(i) -= JxW_face[qp] * (_vector_phi_face[i][qp] * normals[qp]) * lm_sol[qp];
}

void
DiffusionLHDGAssemblyHelper::vectorFaceJacobian(const MooseArray<Real> & JxW_face,
                                                const QBase & qrule_face,
                                                const MooseArray<Point> & normals,
                                                DenseMatrix<Number> & vector_lm_jac)
{
  // Vector equation dependence on LM dofs
  for (const auto i : make_range(vector_lm_jac.m()))
    for (const auto j : make_range(vector_lm_jac.n()))
      for (const auto qp : make_range(qrule_face.n_points()))
        vector_lm_jac(i, j) -=
            JxW_face[qp] * (_vector_phi_face[i][qp] * normals[qp]) * _lm_phi_face[j][qp];
}

void
DiffusionLHDGAssemblyHelper::scalarFaceResidual(const MooseArray<Gradient> & vector_sol,
                                                const MooseArray<Number> & scalar_sol,
                                                const MooseArray<Number> & lm_sol,
                                                const MooseArray<Real> & JxW_face,
                                                const QBase & qrule_face,
                                                const MooseArray<Point> & normals,
                                                DenseVector<Number> & scalar_re)
{
  for (const auto i : index_range(scalar_re))
    for (const auto qp : make_range(qrule_face.n_points()))
    {
      // vector
      scalar_re(i) -=
          JxW_face[qp] * _diff[qp] * _scalar_phi_face[i][qp] * (vector_sol[qp] * normals[qp]);

      // scalar from stabilization term
      scalar_re(i) += JxW_face[qp] * _scalar_phi_face[i][qp] * _tau * scalar_sol[qp] * normals[qp] *
                      normals[qp];

      // lm from stabilization term
      scalar_re(i) -=
          JxW_face[qp] * _scalar_phi_face[i][qp] * _tau * lm_sol[qp] * normals[qp] * normals[qp];
    }
}

void
DiffusionLHDGAssemblyHelper::scalarFaceJacobian(const MooseArray<Real> & JxW_face,
                                                const QBase & qrule_face,
                                                const MooseArray<Point> & normals,
                                                DenseMatrix<Number> & scalar_vector_jac,
                                                DenseMatrix<Number> & scalar_scalar_jac,
                                                DenseMatrix<Number> & scalar_lm_jac)
{
  for (const auto i : make_range(scalar_vector_jac.m()))
    for (const auto qp : make_range(qrule_face.n_points()))
    {
      for (const auto j : make_range(scalar_vector_jac.n()))
        scalar_vector_jac(i, j) -= JxW_face[qp] * _diff[qp] * _scalar_phi_face[i][qp] *
                                   (_vector_phi_face[j][qp] * normals[qp]);

      for (const auto j : make_range(scalar_scalar_jac.n()))
        scalar_scalar_jac(i, j) += JxW_face[qp] * _scalar_phi_face[i][qp] * _tau *
                                   _scalar_phi_face[j][qp] * normals[qp] * normals[qp];

      for (const auto j : make_range(scalar_lm_jac.n()))
        // from stabilization term
        scalar_lm_jac(i, j) -= JxW_face[qp] * _scalar_phi_face[i][qp] * _tau * _lm_phi_face[j][qp] *
                               normals[qp] * normals[qp];
    }
}

void
DiffusionLHDGAssemblyHelper::lmFaceResidual(const MooseArray<Gradient> & vector_sol,
                                            const MooseArray<Number> & scalar_sol,
                                            const MooseArray<Number> & lm_sol,
                                            const MooseArray<Real> & JxW_face,
                                            const QBase & qrule_face,
                                            const MooseArray<Point> & normals,
                                            DenseVector<Number> & lm_re)
{
  for (const auto i : index_range(lm_re))
    for (const auto qp : make_range(qrule_face.n_points()))
    {
      // vector
      lm_re(i) -= JxW_face[qp] * _diff[qp] * _lm_phi_face[i][qp] * (vector_sol[qp] * normals[qp]);

      // scalar from stabilization term
      lm_re(i) +=
          JxW_face[qp] * _lm_phi_face[i][qp] * _tau * scalar_sol[qp] * normals[qp] * normals[qp];

      // lm from stabilization term
      lm_re(i) -=
          JxW_face[qp] * _lm_phi_face[i][qp] * _tau * lm_sol[qp] * normals[qp] * normals[qp];
    }
}

void
DiffusionLHDGAssemblyHelper::lmFaceJacobian(const MooseArray<Real> & JxW_face,
                                            const QBase & qrule_face,
                                            const MooseArray<Point> & normals,
                                            DenseMatrix<Number> & lm_vec_jac,
                                            DenseMatrix<Number> & lm_scalar_jac,
                                            DenseMatrix<Number> & lm_lm_jac)
{
  for (const auto i : make_range(lm_vec_jac.m()))
    for (const auto qp : make_range(qrule_face.n_points()))
    {
      for (const auto j : make_range(lm_vec_jac.n()))
        lm_vec_jac(i, j) -= JxW_face[qp] * _diff[qp] * _lm_phi_face[i][qp] *
                            (_vector_phi_face[j][qp] * normals[qp]);

      for (const auto j : make_range(lm_scalar_jac.n()))
        lm_scalar_jac(i, j) += JxW_face[qp] * _lm_phi_face[i][qp] * _tau * _scalar_phi_face[j][qp] *
                               normals[qp] * normals[qp];

      for (const auto j : make_range(lm_lm_jac.n()))
        // from stabilization term
        lm_lm_jac(i, j) -= JxW_face[qp] * _lm_phi_face[i][qp] * _tau * _lm_phi_face[j][qp] *
                           normals[qp] * normals[qp];
    }
}

void
DiffusionLHDGAssemblyHelper::vectorDirichletResidual(const Moose::Functor<Real> & dirichlet_value,
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
        _ti.determineState());

    // External boundary -> Dirichlet faces -> Vector equation RHS
    for (const auto i : index_range(_qu_dof_indices))
      vector_re(i) -= JxW_face[qp] * (_vector_phi_face[i][qp] * normals[qp]) * scalar_value;
  }
}

void
DiffusionLHDGAssemblyHelper::scalarDirichletResidual(const MooseArray<Gradient> & vector_sol,
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
        _ti.determineState());

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
DiffusionLHDGAssemblyHelper::scalarDirichletJacobian(const MooseArray<Real> & JxW_face,
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
DiffusionLHDGAssemblyHelper::createIdentityResidual(const MooseArray<Real> & JxW,
                                                    const QBase & qrule,
                                                    const MooseArray<std::vector<Real>> & phi,
                                                    const MooseArray<Number> & sol,
                                                    DenseVector<Number> & re)
{
  for (const auto qp : make_range(qrule.n_points()))
    for (const auto i : index_range(phi))
      re(i) -= JxW[qp] * phi[i][qp] * sol[qp];
}

void
DiffusionLHDGAssemblyHelper::createIdentityJacobian(const MooseArray<Real> & JxW,
                                                    const QBase & qrule,
                                                    const MooseArray<std::vector<Real>> & phi,
                                                    DenseMatrix<Number> & ke)
{
  for (const auto qp : make_range(qrule.n_points()))
    for (const auto i : index_range(phi))
      for (const auto j : index_range(phi))
        ke(i, j) -= JxW[qp] * phi[i][qp] * phi[j][qp];
}
