//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiffusionHDGAssemblyHelper.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "Function.h"
#include "SystemBase.h"
#include "MooseMesh.h"
#include "MooseObject.h"
#include "MaterialPropertyInterface.h"

using namespace libMesh;

InputParameters
DiffusionHDGAssemblyHelper::validParams()
{
  auto params = emptyInputParameters();
  params.addRequiredParam<AuxVariableName>("u", "The diffusing specie concentration");
  params.addRequiredParam<AuxVariableName>("grad_u",
                                           "The gradient of the diffusing specie concentration");
  params.addRequiredParam<NonlinearVariableName>(
      "face_u", "The concentration of the diffusing specie on faces");
  params.addRequiredParam<MaterialPropertyName>("diffusivity", "The diffusivity");
  params.addParam<Real>("tau",
                        1,
                        "The stabilization coefficient required for discontinuous Galerkin "
                        "schemes. This may be set to 0 for a mixed method with Raviart-Thomas.");
  return params;
}

DiffusionHDGAssemblyHelper::DiffusionHDGAssemblyHelper(const MooseObject * const moose_obj,
                                                       MaterialPropertyInterface * const mpi,
                                                       const TransientInterface * const ti,
                                                       SystemBase & nl_sys,
                                                       SystemBase & aux_sys,
                                                       const THREAD_ID tid)
  : // vars
    _u_var(aux_sys.getFieldVariable<Real>(tid, moose_obj->getParam<AuxVariableName>("u"))),
    _grad_u_var(aux_sys.getFieldVariable<RealVectorValue>(
        tid, moose_obj->getParam<AuxVariableName>("grad_u"))),
    _u_face_var(
        nl_sys.getFieldVariable<Real>(tid, moose_obj->getParam<NonlinearVariableName>("face_u"))),
    // dof indices
    _qu_dof_indices(_grad_u_var.dofIndices()),
    _u_dof_indices(_u_var.dofIndices()),
    _lm_u_dof_indices(_u_face_var.dofIndices()),
    // solutions
    _qu_sol(_grad_u_var.sln()),
    _u_sol(_u_var.sln()),
    _lm_u_sol(_u_face_var.sln()),
    // shape functions
    _vector_phi(_grad_u_var.phi()),
    _scalar_phi(_u_var.phi()),
    _grad_scalar_phi(_u_var.gradPhi()),
    _div_vector_phi(_grad_u_var.divPhi()),
    _vector_phi_face(_grad_u_var.phiFace()),
    _scalar_phi_face(_u_var.phiFace()),
    _lm_phi_face(_u_face_var.phiFace()),
    // material properties
    _diff(mpi->getMaterialProperty<Real>("diffusivity")),
    // initialize local number of dofs
    _vector_n_dofs(0),
    _scalar_n_dofs(0),
    _lm_n_dofs(0),
    _ti(*ti),
    _tau(moose_obj->getParam<Real>("tau"))
{
}

std::set<const MooseVariableBase *>
DiffusionHDGAssemblyHelper::variables() const
{
  return {&_u_var, &_grad_u_var, &_u_face_var};
}

void
DiffusionHDGAssemblyHelper::resizeData()
{
  _vector_n_dofs = _qu_dof_indices.size();
  _scalar_n_dofs = _u_dof_indices.size();
  _lm_n_dofs = _lm_u_dof_indices.size();

  libmesh_assert_equal_to(_vector_n_dofs, _vector_phi.size());
  libmesh_assert_equal_to(_scalar_n_dofs, _scalar_phi.size());

  _primal_size = _vector_n_dofs + _scalar_n_dofs;
  _lm_size = _lm_n_dofs;

  // prepare our matrix/vector data structures
  _PrimalMat.setZero(_primal_size, _primal_size);
  _PrimalVec.setZero(_primal_size);
  _LMMat.setZero(_lm_size, _lm_size);
  _LMVec.setZero(_lm_size);
  _PrimalLM.setZero(_primal_size, _lm_size);
  _LMPrimal.setZero(_lm_size, _primal_size);
}

void
DiffusionHDGAssemblyHelper::vectorVolumeResidual(const unsigned int i_offset,
                                                 const MooseArray<Gradient> & vector_sol,
                                                 const MooseArray<Number> & scalar_sol,
                                                 const MooseArray<Real> & JxW,
                                                 const QBase & qrule)
{
  for (const auto i : make_range(_vector_n_dofs))
    for (const auto qp : make_range(qrule.n_points()))
    {
      // Vector equation dependence on vector dofs
      _PrimalVec(i_offset + i) += JxW[qp] * (_vector_phi[i][qp] * vector_sol[qp]);

      // Vector equation dependence on scalar dofs
      _PrimalVec(i_offset + i) += JxW[qp] * (_div_vector_phi[i][qp] * scalar_sol[qp]);
    }
}

void
DiffusionHDGAssemblyHelper::vectorVolumeJacobian(const unsigned int i_offset,
                                                 const unsigned int vector_j_offset,
                                                 const unsigned int scalar_j_offset,
                                                 const MooseArray<Real> & JxW,
                                                 const QBase & qrule)
{
  for (const auto i : make_range(_vector_n_dofs))
    for (const auto qp : make_range(qrule.n_points()))
    {
      // Vector equation dependence on vector dofs
      for (const auto j : make_range(_vector_n_dofs))
        _PrimalMat(i_offset + i, vector_j_offset + j) +=
            JxW[qp] * (_vector_phi[i][qp] * _vector_phi[j][qp]);

      // Vector equation dependence on scalar dofs
      for (const auto j : make_range(_scalar_n_dofs))
        _PrimalMat(i_offset + i, scalar_j_offset + j) +=
            JxW[qp] * (_div_vector_phi[i][qp] * _scalar_phi[j][qp]);
    }
}

void
DiffusionHDGAssemblyHelper::scalarVolumeResidual(const unsigned int i_offset,
                                                 const MooseArray<Gradient> & vector_field,
                                                 const Moose::Functor<Real> & source,
                                                 const MooseArray<Real> & JxW,
                                                 const QBase & qrule,
                                                 const Elem * const current_elem,
                                                 const MooseArray<Point> & q_point)
{
  for (const auto qp : make_range(qrule.n_points()))
  {
    // Evaluate source
    const auto f =
        source(Moose::ElemQpArg{current_elem, qp, &qrule, q_point[qp]}, _ti.determineState());

    for (const auto i : make_range(_scalar_n_dofs))
    {
      _PrimalVec(i_offset + i) +=
          JxW[qp] * (_grad_scalar_phi[i][qp] * _diff[qp] * vector_field[qp]);

      // Scalar equation RHS
      _PrimalVec(i_offset + i) -= JxW[qp] * _scalar_phi[i][qp] * f;
    }
  }
}

void
DiffusionHDGAssemblyHelper::scalarVolumeJacobian(const unsigned int i_offset,
                                                 const unsigned int vector_field_j_offset,
                                                 const MooseArray<Real> & JxW,
                                                 const QBase & qrule)
{
  for (const auto i : make_range(_scalar_n_dofs))
    // Scalar equation dependence on vector dofs
    for (const auto j : make_range(_vector_n_dofs))
      for (const auto qp : make_range(qrule.n_points()))
        _PrimalMat(i_offset + i, vector_field_j_offset + j) +=
            JxW[qp] * _diff[qp] * (_grad_scalar_phi[i][qp] * _vector_phi[j][qp]);
}

void
DiffusionHDGAssemblyHelper::vectorFaceResidual(const unsigned int i_offset,
                                               const MooseArray<Number> & lm_sol,
                                               const MooseArray<Real> & JxW_face,
                                               const QBase & qrule_face,
                                               const MooseArray<Point> & normals)
{
  // Vector equation dependence on LM dofs
  for (const auto i : make_range(_vector_n_dofs))
    for (const auto qp : make_range(qrule_face.n_points()))
      _PrimalVec(i_offset + i) -=
          JxW_face[qp] * (_vector_phi_face[i][qp] * normals[qp]) * lm_sol[qp];
}

void
DiffusionHDGAssemblyHelper::vectorFaceJacobian(const unsigned int i_offset,
                                               const unsigned int lm_j_offset,
                                               const MooseArray<Real> & JxW_face,
                                               const QBase & qrule_face,
                                               const MooseArray<Point> & normals)
{
  // Vector equation dependence on LM dofs
  for (const auto i : make_range(_vector_n_dofs))
    for (const auto j : make_range(_lm_n_dofs))
      for (const auto qp : make_range(qrule_face.n_points()))
        _PrimalLM(i_offset + i, lm_j_offset + j) -=
            JxW_face[qp] * (_vector_phi_face[i][qp] * normals[qp]) * _lm_phi_face[j][qp];
}

void
DiffusionHDGAssemblyHelper::scalarFaceResidual(const unsigned int i_offset,
                                               const MooseArray<Gradient> & vector_sol,
                                               const MooseArray<Number> & scalar_sol,
                                               const MooseArray<Number> & lm_sol,
                                               const MooseArray<Real> & JxW_face,
                                               const QBase & qrule_face,
                                               const MooseArray<Point> & normals)
{
  for (const auto i : make_range(_scalar_n_dofs))
    for (const auto qp : make_range(qrule_face.n_points()))
    {
      // vector
      _PrimalVec(i_offset + i) -=
          JxW_face[qp] * _diff[qp] * _scalar_phi_face[i][qp] * (vector_sol[qp] * normals[qp]);

      // scalar from stabilization term
      _PrimalVec(i_offset + i) += JxW_face[qp] * _scalar_phi_face[i][qp] * _tau * scalar_sol[qp] *
                                  normals[qp] * normals[qp];

      // lm from stabilization term
      _PrimalVec(i_offset + i) -=
          JxW_face[qp] * _scalar_phi_face[i][qp] * _tau * lm_sol[qp] * normals[qp] * normals[qp];
    }
}

void
DiffusionHDGAssemblyHelper::scalarFaceJacobian(const unsigned int i_offset,
                                               const unsigned int vector_j_offset,
                                               const unsigned int scalar_j_offset,
                                               const unsigned int lm_j_offset,
                                               const MooseArray<Real> & JxW_face,
                                               const QBase & qrule_face,
                                               const MooseArray<Point> & normals)
{
  for (const auto i : make_range(_scalar_n_dofs))
    for (const auto qp : make_range(qrule_face.n_points()))
    {
      for (const auto j : make_range(_vector_n_dofs))
        _PrimalMat(i_offset + i, vector_j_offset + j) -= JxW_face[qp] * _diff[qp] *
                                                         _scalar_phi_face[i][qp] *
                                                         (_vector_phi_face[j][qp] * normals[qp]);

      for (const auto j : make_range(_scalar_n_dofs))
        _PrimalMat(i_offset + i, scalar_j_offset + j) += JxW_face[qp] * _scalar_phi_face[i][qp] *
                                                         _tau * _scalar_phi_face[j][qp] *
                                                         normals[qp] * normals[qp];

      for (const auto j : make_range(_lm_n_dofs))
        // from stabilization term
        _PrimalLM(i_offset + i, lm_j_offset + j) -= JxW_face[qp] * _scalar_phi_face[i][qp] * _tau *
                                                    _lm_phi_face[j][qp] * normals[qp] * normals[qp];
    }
}

void
DiffusionHDGAssemblyHelper::lmFaceResidual(const unsigned int i_offset,
                                           const MooseArray<Gradient> & vector_sol,
                                           const MooseArray<Number> & scalar_sol,
                                           const MooseArray<Number> & lm_sol,
                                           const MooseArray<Real> & JxW_face,
                                           const QBase & qrule_face,
                                           const MooseArray<Point> & normals)
{
  for (const auto i : make_range(_lm_n_dofs))
    for (const auto qp : make_range(qrule_face.n_points()))
    {
      // vector
      _LMVec(i_offset + i) -=
          JxW_face[qp] * _diff[qp] * _lm_phi_face[i][qp] * (vector_sol[qp] * normals[qp]);

      // scalar from stabilization term
      _LMVec(i_offset + i) +=
          JxW_face[qp] * _lm_phi_face[i][qp] * _tau * scalar_sol[qp] * normals[qp] * normals[qp];

      // lm from stabilization term
      _LMVec(i_offset + i) -=
          JxW_face[qp] * _lm_phi_face[i][qp] * _tau * lm_sol[qp] * normals[qp] * normals[qp];
    }
}

void
DiffusionHDGAssemblyHelper::lmFaceJacobian(const unsigned int i_offset,
                                           const unsigned int vector_j_offset,
                                           const unsigned int scalar_j_offset,
                                           const unsigned int lm_j_offset,
                                           const MooseArray<Real> & JxW_face,
                                           const QBase & qrule_face,
                                           const MooseArray<Point> & normals)
{
  for (const auto i : make_range(_lm_n_dofs))
    for (const auto qp : make_range(qrule_face.n_points()))
    {
      for (const auto j : make_range(_vector_n_dofs))
        _LMPrimal(i_offset + i, vector_j_offset + j) -= JxW_face[qp] * _diff[qp] *
                                                        _lm_phi_face[i][qp] *
                                                        (_vector_phi_face[j][qp] * normals[qp]);

      for (const auto j : make_range(_scalar_n_dofs))
        _LMPrimal(i_offset + i, scalar_j_offset + j) += JxW_face[qp] * _lm_phi_face[i][qp] * _tau *
                                                        _scalar_phi_face[j][qp] * normals[qp] *
                                                        normals[qp];

      for (const auto j : make_range(_lm_n_dofs))
        // from stabilization term
        _LMMat(i_offset + i, lm_j_offset + j) -= JxW_face[qp] * _lm_phi_face[i][qp] * _tau *
                                                 _lm_phi_face[j][qp] * normals[qp] * normals[qp];
    }
}

void
DiffusionHDGAssemblyHelper::vectorDirichletResidual(const unsigned int i_offset,
                                                    const Moose::Functor<Real> & dirichlet_value,
                                                    const MooseArray<Real> & JxW_face,
                                                    const QBase & qrule_face,
                                                    const MooseArray<Point> & normals,
                                                    const Elem * const current_elem,
                                                    const unsigned int current_side,
                                                    const MooseArray<Point> & q_point_face)
{
  for (const auto qp : make_range(qrule_face.n_points()))
  {
    const auto scalar_value = dirichlet_value(
        Moose::ElemSideQpArg{current_elem, current_side, qp, &qrule_face, q_point_face[qp]},
        _ti.determineState());

    // External boundary -> Dirichlet faces -> Vector equation RHS
    for (const auto i : make_range(_vector_n_dofs))
      _PrimalVec(i_offset + i) -=
          JxW_face[qp] * (_vector_phi_face[i][qp] * normals[qp]) * scalar_value;
  }
}

void
DiffusionHDGAssemblyHelper::scalarDirichletResidual(const unsigned int i_offset,
                                                    const MooseArray<Gradient> & vector_sol,
                                                    const MooseArray<Number> & scalar_sol,
                                                    const Moose::Functor<Real> & dirichlet_value,
                                                    const MooseArray<Real> & JxW_face,
                                                    const QBase & qrule_face,
                                                    const MooseArray<Point> & normals,
                                                    const Elem * const current_elem,
                                                    const unsigned int current_side,
                                                    const MooseArray<Point> & q_point_face)
{
  for (const auto qp : make_range(qrule_face.n_points()))
  {
    const auto scalar_value = dirichlet_value(
        Moose::ElemSideQpArg{current_elem, current_side, qp, &qrule_face, q_point_face[qp]},
        _ti.determineState());

    for (const auto i : make_range(_scalar_n_dofs))
    {
      // vector
      _PrimalVec(i_offset + i) -=
          JxW_face[qp] * _diff[qp] * _scalar_phi_face[i][qp] * (vector_sol[qp] * normals[qp]);

      // scalar from stabilization term
      _PrimalVec(i_offset + i) += JxW_face[qp] * _scalar_phi_face[i][qp] * _tau * scalar_sol[qp] *
                                  normals[qp] * normals[qp];

      // dirichlet lm from stabilization term
      _PrimalVec(i_offset + i) -=
          JxW_face[qp] * _scalar_phi_face[i][qp] * _tau * scalar_value * normals[qp] * normals[qp];
    }
  }
}

void
DiffusionHDGAssemblyHelper::scalarDirichletJacobian(const unsigned int i_offset,
                                                    const unsigned int vector_j_offset,
                                                    const unsigned int scalar_j_offset,
                                                    const MooseArray<Real> & JxW_face,
                                                    const QBase & qrule_face,
                                                    const MooseArray<Point> & normals)
{
  for (const auto i : make_range(_scalar_n_dofs))
    for (const auto qp : make_range(qrule_face.n_points()))
    {
      for (const auto j : make_range(_vector_n_dofs))
        _PrimalMat(i_offset + i, vector_j_offset + j) -= JxW_face[qp] * _diff[qp] *
                                                         _scalar_phi_face[i][qp] *
                                                         (_vector_phi_face[j][qp] * normals[qp]);

      for (const auto j : make_range(_scalar_n_dofs))
        _PrimalMat(i_offset + i, scalar_j_offset + j) += JxW_face[qp] * _scalar_phi_face[i][qp] *
                                                         _tau * _scalar_phi_face[j][qp] *
                                                         normals[qp] * normals[qp];
    }
}
