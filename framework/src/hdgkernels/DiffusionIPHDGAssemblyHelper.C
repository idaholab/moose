//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiffusionIPHDGAssemblyHelper.h"
#include "MooseTypes.h"
#include "MooseVariableDependencyInterface.h"
#include "MooseVariableScalar.h"
#include "SystemBase.h"
#include "MooseMesh.h"
#include "MooseObject.h"
#include "MaterialPropertyInterface.h"

using namespace libMesh;

InputParameters
DiffusionIPHDGAssemblyHelper::validParams()
{
  auto params = IPHDGAssemblyHelper::validParams();
  params.addRequiredParam<MaterialPropertyName>("diffusivity", "The diffusivity");
  params.addParam<Real>("alpha",
                        1,
                        "The stabilization coefficient required for discontinuous Galerkin "
                        "schemes.");
  return params;
}

DiffusionIPHDGAssemblyHelper::DiffusionIPHDGAssemblyHelper(
    const MooseObject * const moose_obj,
    MooseVariableDependencyInterface * const mvdi,
    const TransientInterface * const ti,
    SystemBase & sys,
    const Assembly & assembly,
    const THREAD_ID tid,
    const std::set<SubdomainID> & block_ids,
    const std::set<BoundaryID> & boundary_ids)
  : IPHDGAssemblyHelper(moose_obj, mvdi, ti, sys, assembly, tid, block_ids, boundary_ids),
    _diff(this->getADMaterialProperty<Real>("diffusivity")),
    _face_diff(this->getFaceADMaterialProperty<Real>("diffusivity")),
    _alpha(moose_obj->getParam<Real>("alpha"))
{
}

void
DiffusionIPHDGAssemblyHelper::scalarVolume()
{
  for (const auto qp : make_range(_ip_qrule->n_points()))
    for (const auto i : index_range(_scalar_re))
      _scalar_re(i) += _ip_JxW[qp] * (_grad_scalar_phi[i][qp] * _diff[qp] * _grad_u_sol[qp]);
}

void
DiffusionIPHDGAssemblyHelper::scalarFace()
{
  const auto h_elem = _elem_volume / _side_area;

  for (const auto i : index_range(_scalar_re))
    for (const auto qp : make_range(_ip_qrule_face->n_points()))
    {
      _scalar_re(i) -= _ip_JxW_face[qp] * _face_diff[qp] * _scalar_phi_face[i][qp] *
                       (_grad_u_sol[qp] * _ip_normals[qp]);
      _scalar_re(i) -= _ip_JxW_face[qp] * _alpha / h_elem * _face_diff[qp] *
                       (_lm_u_sol[qp] - _u_sol[qp]) * _scalar_phi_face[i][qp];
      _scalar_re(i) += _ip_JxW_face[qp] * (_lm_u_sol[qp] - _u_sol[qp]) * _face_diff[qp] *
                       _grad_scalar_phi_face[i][qp] * _ip_normals[qp];
    }
}

void
DiffusionIPHDGAssemblyHelper::lmFace()
{
  const auto h_elem = _elem_volume / _side_area;

  for (const auto i : index_range(_lm_re))
    for (const auto qp : make_range(_ip_qrule_face->n_points()))
    {
      _lm_re(i) += _ip_JxW_face[qp] * _face_diff[qp] * _grad_u_sol[qp] * _ip_normals[qp] *
                   _lm_phi_face[i][qp];
      _lm_re(i) += _ip_JxW_face[qp] * _alpha / h_elem * _face_diff[qp] *
                   (_lm_u_sol[qp] - _u_sol[qp]) * _lm_phi_face[i][qp];
    }
}

void
DiffusionIPHDGAssemblyHelper::scalarDirichlet(const Moose::Functor<Real> & dirichlet_value)
{
  const auto h_elem = _elem_volume / _side_area;

  for (const auto qp : make_range(_ip_qrule_face->n_points()))
  {
    const auto scalar_value = dirichlet_value(
        Moose::ElemSideQpArg{
            _ip_current_elem, _ip_current_side, qp, _ip_qrule_face, _ip_q_point_face[qp]},
        _ti.determineState());

    for (const auto i : index_range(_u_dof_indices))
    {
      _scalar_re(i) -= _ip_JxW_face[qp] * _face_diff[qp] * _scalar_phi_face[i][qp] *
                       (_grad_u_sol[qp] * _ip_normals[qp]);
      _scalar_re(i) -= _ip_JxW_face[qp] * _alpha / h_elem * _face_diff[qp] *
                       (scalar_value - _u_sol[qp]) * _scalar_phi_face[i][qp];
      _scalar_re(i) += _ip_JxW_face[qp] * (scalar_value - _u_sol[qp]) * _face_diff[qp] *
                       _grad_scalar_phi_face[i][qp] * _ip_normals[qp];
    }
  }
}
