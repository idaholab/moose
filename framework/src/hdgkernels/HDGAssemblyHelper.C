//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HDGAssemblyHelper.h"
#include "MooseVariableDependencyInterface.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"
#include "MooseObject.h"
#include "TransientInterface.h"

using namespace libMesh;

InputParameters
HDGAssemblyHelper::validParams()
{
  auto params = emptyInputParameters();
  params.addRequiredParam<NonlinearVariableName>("face_variable", "The face variable");
  return params;
}

HDGAssemblyHelper::HDGAssemblyHelper(
    const MooseObject * const moose_obj,
    MooseVariableDependencyInterface * const mvdi,
    const TransientInterface * const ti,
    SystemBase & sys,
    const Assembly & assembly,
    const THREAD_ID tid,
    const std::set<SubdomainID> & block_ids,
    const std::set<BoundaryID> & boundary_ids)
  : ThreeMaterialPropertyInterface(moose_obj, block_ids, boundary_ids),
    _ti(*ti),
    _u_var(sys.getFieldVariable<Real>(tid, moose_obj->getParam<NonlinearVariableName>("variable"))),
    _u_face_var(sys.getFieldVariable<Real>(
        tid, moose_obj->getParam<NonlinearVariableName>("face_variable"))),
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
    _elem_volume(assembly.elemVolume()),
    _side_area(assembly.sideElemVolume()),
    _current_elem(assembly.elem()),
    _current_side(assembly.side()),
    _current_side_elem(assembly.sideElem()),
    _JxW(assembly.JxW()),
    _qrule(assembly.qRule()),
    _q_point(assembly.qPoints()),
    _JxW_face(assembly.JxWFace()),
    _qrule_face(assembly.qRuleFace()),
    _q_point_face(assembly.qPointsFace()),
    _normals(assembly.normals())
{
  mvdi->addMooseVariableDependency(&const_cast<MooseVariableFE<Real> &>(_u_var));
  mvdi->addMooseVariableDependency(&const_cast<MooseVariableFE<Real> &>(_u_face_var));
}

std::array<ADResidualsPacket, 2>
HDGAssemblyHelper::taggingData() const
{
  return {ADResidualsPacket{_scalar_re, _u_dof_indices, _u_var.scalingFactor()},
          ADResidualsPacket{_lm_re, _lm_u_dof_indices, _u_face_var.scalingFactor()}};
}

std::set<std::string>
HDGAssemblyHelper::additionalROVariables()
{
  return {_u_face_var.name()};
}

void
HDGAssemblyHelper::lmDirichlet(const Moose::Functor<Real> & dirichlet_value)
{
  for (const auto qp : make_range(_qrule_face->n_points()))
  {
    const auto scalar_value = dirichlet_value(
        Moose::ElemSideQpArg{
            _current_elem, _current_side, qp, _qrule_face, _q_point_face[qp]},
        _ti.determineState());

    for (const auto i : index_range(_lm_re))
      _lm_re(i) += _JxW_face[qp] * (_lm_u_sol[qp] - scalar_value) * _lm_phi_face[i][qp];
  }
}

void
HDGAssemblyHelper::lmPrescribedFlux(const Moose::Functor<Real> & flux_value)
{
  for (const auto qp : make_range(_qrule_face->n_points()))
  {
    const auto flux = flux_value(
        Moose::ElemSideQpArg{
            _current_elem, _current_side, qp, _qrule_face, _q_point_face[qp]},
        _ti.determineState());

    for (const auto i : index_range(_lm_re))
      _lm_re(i) += _JxW_face[qp] * flux * _lm_phi_face[i][qp];
  }
}
