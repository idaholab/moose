//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IPHDGAssemblyHelper.h"
#include "MooseTypes.h"
#include "MooseVariableDependencyInterface.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "SystemBase.h"
#include "MooseMesh.h"
#include "MooseObject.h"
#include "MaterialPropertyInterface.h"
#include "TransientInterface.h"

using namespace libMesh;

InputParameters
IPHDGAssemblyHelper::validParams()
{
  auto params = emptyInputParameters();
  params.addRequiredParam<NonlinearVariableName>("face_variable", "The face variable");
  return params;
}

IPHDGAssemblyHelper::IPHDGAssemblyHelper(const MooseObject * const moose_obj,
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
    _ip_current_elem(assembly.elem()),
    _ip_current_side(assembly.side()),
    _ip_JxW(assembly.JxW()),
    _ip_qrule(assembly.qRule()),
    _ip_q_point(assembly.qPoints()),
    _ip_JxW_face(assembly.JxWFace()),
    _ip_qrule_face(assembly.qRuleFace()),
    _ip_q_point_face(assembly.qPointsFace()),
    _ip_normals(assembly.normals())
{
  mvdi->addMooseVariableDependency(&const_cast<MooseVariableFE<Real> &>(_u_var));
  mvdi->addMooseVariableDependency(&const_cast<MooseVariableFE<Real> &>(_u_face_var));
}

std::array<ADResidualsPacket, 2>
IPHDGAssemblyHelper::taggingData() const
{
  return {ADResidualsPacket{_scalar_re, _u_dof_indices, _u_var.scalingFactor()},
          ADResidualsPacket{_lm_re, _lm_u_dof_indices, _u_face_var.scalingFactor()}};
}

std::set<std::string>
IPHDGAssemblyHelper::variablesCovered()
{
  return {_u_var.name(), _u_face_var.name()};
}

void
IPHDGAssemblyHelper::lmDirichlet(const Moose::Functor<Real> & dirichlet_value)
{
  for (const auto qp : make_range(_ip_qrule_face->n_points()))
  {
    const auto scalar_value = dirichlet_value(
        Moose::ElemSideQpArg{
            _ip_current_elem, _ip_current_side, qp, _ip_qrule_face, _ip_q_point_face[qp]},
        _ti.determineState());

    for (const auto i : index_range(_lm_re))
      _lm_re(i) += _ip_JxW_face[qp] * (_lm_u_sol[qp] - scalar_value) * _lm_phi_face[i][qp];
  }
}

void
IPHDGAssemblyHelper::lmPrescribedFlux(const Moose::Functor<Real> & flux_value)
{
  for (const auto qp : make_range(_ip_qrule_face->n_points()))
  {
    const auto flux = flux_value(
        Moose::ElemSideQpArg{
            _ip_current_elem, _ip_current_side, qp, _ip_qrule_face, _ip_q_point_face[qp]},
        _ti.determineState());

    for (const auto i : index_range(_lm_re))
      _lm_re(i) += _ip_JxW_face[qp] * flux * _lm_phi_face[i][qp];
  }
}
