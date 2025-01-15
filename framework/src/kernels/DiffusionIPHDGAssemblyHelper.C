//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ActionWarehouse.h"
#include "DiffusionIPHDGAssemblyHelper.h"
#include "MooseTypes.h"
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
DiffusionIPHDGAssemblyHelper::validParams()
{
  auto params = emptyInputParameters();
  params.addRequiredParam<NonlinearVariableName>(
      "face_u", "The concentration of the _diffusing specie on faces");
  params.addRequiredParam<MaterialPropertyName>("diffusivity", "The diffusivity");
  params.addParam<Real>("alpha",
                        1,
                        "The stabilization coefficient required for discontinuous Galerkin "
                        "schemes.");
  return params;
}

DiffusionIPHDGAssemblyHelper::DiffusionIPHDGAssemblyHelper(
    const MooseObject * const moose_obj,
    MaterialPropertyInterface * const mpi,
    MooseVariableDependencyInterface * const mvdi,
    const TransientInterface * const ti,
    SystemBase & sys,
    const Assembly & assembly,
    const THREAD_ID tid)
  : ADFunctorInterface(moose_obj),
    _u_var(sys.getFieldVariable<Real>(tid, moose_obj->getParam<NonlinearVariableName>("u"))),
    _u_face_var(
        sys.getFieldVariable<Real>(tid, moose_obj->getParam<NonlinearVariableName>("face_u"))),
    _u_dof_indices(_u_var.dofIndices()),
    _lm_u_dof_indices(_u_face_var.dofIndices()),
    _u_sol(_u_var.adSln()),
    _grad_u_sol(_u_var.adGradSln()),
    _lm_u_sol(_u_face_var.adSln()),
    _scalar_phi(_u_var.phi()),
    _grad_scalar_phi(_u_var.gradPhi()),
    _scalar_phi_face(_u_var.phiFace()),
    _grad_scalar_phi_face(_u_var.gradPhiFace()),
    _lm_phi_face(_u_face_var.phiFace()),
    _diff(mpi->getMaterialProperty<Real>("diffusivity")),
    _ti(*ti),
    _alpha(moose_obj->getParam<Real>("alpha")),
    _elem_volume(assembly.elemVolume()),
    _side_area(assembly.sideElemVolume()),
    _my_elem(nullptr)
{
  mvdi->addMooseVariableDependency(&const_cast<MooseVariableFE<Real> &>(_u_var));
  mvdi->addMooseVariableDependency(&const_cast<MooseVariableFE<Real> &>(_u_face_var));
}

void
DiffusionIPHDGAssemblyHelper::scalarVolume(const MooseArray<ADRealVectorValue> & grad_scalar_sol,
                                           const Moose::Functor<Real> & source,
                                           const MooseArray<Real> & JxW,
                                           const QBase & qrule,
                                           const Elem * const current_elem,
                                           const MooseArray<Point> & q_point,
                                           DenseVector<ADReal> & scalar_re)
{
  for (const auto qp : make_range(qrule.n_points()))
  {
    // Evaluate source
    const auto f =
        source(Moose::ElemQpArg{current_elem, qp, &qrule, q_point[qp]}, _ti.determineState());

    for (const auto i : index_range(scalar_re))
    {
      scalar_re(i) += JxW[qp] * (_grad_scalar_phi[i][qp] * _diff[qp] * grad_scalar_sol[qp]);

      // Scalar equation RHS
      scalar_re(i) -= JxW[qp] * _scalar_phi[i][qp] * f;
    }
  }
}

void
DiffusionIPHDGAssemblyHelper::scalarFace(const MooseArray<ADRealVectorValue> & grad_scalar_sol,
                                         const MooseArray<ADReal> & scalar_sol,
                                         const MooseArray<ADReal> & lm_sol,
                                         const MooseArray<Real> & JxW_face,
                                         const QBase & qrule_face,
                                         const MooseArray<Point> & normals,
                                         DenseVector<ADReal> & scalar_re)
{
  const auto h_elem = _elem_volume / _side_area;

  for (const auto i : index_range(scalar_re))
    for (const auto qp : make_range(qrule_face.n_points()))
    {
      scalar_re(i) -=
          JxW_face[qp] * _diff[qp] * _scalar_phi_face[i][qp] * (grad_scalar_sol[qp] * normals[qp]);
      scalar_re(i) -=
          _alpha / h_elem * _diff[qp] * (lm_sol[qp] - scalar_sol[qp]) * _scalar_phi_face[i][qp];
      scalar_re(i) +=
          (lm_sol[qp] - scalar_sol[qp]) * _diff[qp] * _grad_scalar_phi_face[i][qp] * normals[qp];
    }
}

void
DiffusionIPHDGAssemblyHelper::lmFace(const MooseArray<ADRealVectorValue> & grad_scalar_sol,
                                     const MooseArray<ADReal> & scalar_sol,
                                     const MooseArray<ADReal> & lm_sol,
                                     const MooseArray<Real> & JxW_face,
                                     const QBase & qrule_face,
                                     const MooseArray<Point> & normals,
                                     DenseVector<ADReal> & lm_re)
{
  const auto h_elem = _elem_volume / _side_area;

  for (const auto i : index_range(lm_re))
    for (const auto qp : make_range(qrule_face.n_points()))
    {
      lm_re(i) +=
          JxW_face[qp] * _diff[qp] * grad_scalar_sol[qp] * normals[qp] * _lm_phi_face[i][qp];
      lm_re(i) += JxW_face[qp] * _alpha / h_elem * _diff[qp] * (lm_sol[qp] - scalar_sol[qp]) *
                  _lm_phi_face[i][qp];
    }
}

void
DiffusionIPHDGAssemblyHelper::scalarDirichlet(const MooseArray<ADRealVectorValue> & grad_scalar_sol,
                                              const MooseArray<ADReal> & lm_sol,
                                              const Moose::Functor<Real> & dirichlet_value,
                                              const MooseArray<Real> & JxW_face,
                                              const QBase & qrule_face,
                                              const MooseArray<Point> & normals,
                                              const Elem * const current_elem,
                                              const unsigned int current_side,
                                              const MooseArray<Point> & q_point_face,
                                              DenseVector<ADReal> & scalar_re)
{
  const auto h_elem = _elem_volume / _side_area;

  for (const auto qp : make_range(qrule_face.n_points()))
  {
    const auto scalar_value = dirichlet_value(
        Moose::ElemSideQpArg{current_elem, current_side, qp, &qrule_face, q_point_face[qp]},
        _ti.determineState());

    for (const auto i : index_range(_u_dof_indices))
    {
      scalar_re(i) -=
          JxW_face[qp] * _diff[qp] * _scalar_phi_face[i][qp] * (grad_scalar_sol[qp] * normals[qp]);
      scalar_re(i) -=
          _alpha / h_elem * _diff[qp] * (lm_sol[qp] - scalar_value) * _scalar_phi_face[i][qp];
      scalar_re(i) +=
          (lm_sol[qp] - scalar_value) * _diff[qp] * _grad_scalar_phi_face[i][qp] * normals[qp];
    }
  }
}

void
DiffusionIPHDGAssemblyHelper::lmDirichlet(const MooseArray<ADRealVectorValue> & grad_scalar_sol,
                                          const MooseArray<ADReal> & lm_sol,
                                          const Moose::Functor<Real> & dirichlet_value,
                                          const MooseArray<Real> & JxW_face,
                                          const QBase & qrule_face,
                                          const MooseArray<Point> & normals,
                                          const Elem * const current_elem,
                                          const unsigned int current_side,
                                          const MooseArray<Point> & q_point_face,
                                          DenseVector<ADReal> & lm_re)
{
  const auto h_elem = _elem_volume / _side_area;

  for (const auto qp : make_range(qrule_face.n_points()))
  {
    const auto scalar_value = dirichlet_value(
        Moose::ElemSideQpArg{current_elem, current_side, qp, &qrule_face, q_point_face[qp]},
        _ti.determineState());

    for (const auto i : index_range(lm_re))
    {
      lm_re(i) +=
          JxW_face[qp] * _diff[qp] * grad_scalar_sol[qp] * normals[qp] * _lm_phi_face[i][qp];
      lm_re(i) += JxW_face[qp] * _alpha / h_elem * _diff[qp] * (lm_sol[qp] - scalar_value) *
                  _lm_phi_face[i][qp];
    }
  }
}
