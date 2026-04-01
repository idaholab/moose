//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MassFluxPenaltyIPHDGAssemblyHelper.h"

// MOOSE includes
#include "MooseVariableFE.h"

InputParameters
MassFluxPenaltyIPHDGAssemblyHelper::validParams()
{
  InputParameters params = IPHDGAssemblyHelper::validParams();
  params.addRequiredParam<NonlinearVariableName>("u", "The x-velocity");
  params.addRequiredParam<NonlinearVariableName>("v", "The y-velocity");
  params.addParam<NonlinearVariableName>("w", "The z-velocity");
  params.addRequiredRangeCheckedParam<unsigned short>(
      "component", "0<=component<=1", "The velocity component this object is being applied to");
  params.addRequiredParam<Real>("gamma", "The penalty to multiply the jump");
  params.addClassDescription("introduces a jump correction on internal faces for grad-div "
                             "stabilization for discontinuous Galerkin methods.");
  params.addRequiredParam<MooseFunctorName>("face_velocity",
                                            "A vector functor representing the face velocity");
  return params;
}

MassFluxPenaltyIPHDGAssemblyHelper::MassFluxPenaltyIPHDGAssemblyHelper(
    const MooseObject * const moose_obj,
    MooseVariableDependencyInterface * const mvdi,
    const TransientInterface * const ti,
    const MooseMesh & mesh,
    SystemBase & sys,
    const Assembly & assembly,
    const THREAD_ID tid,
    const std::set<SubdomainID> & block_ids,
    const std::set<BoundaryID> & boundary_ids)
  : IPHDGAssemblyHelper(moose_obj, mvdi, ti, sys, assembly, tid, block_ids, boundary_ids),
    ADFunctorInterface(moose_obj),
    _vel_x_var(sys.getFieldVariable<Real>(tid, moose_obj->getParam<NonlinearVariableName>("u"))),
    _vel_y_var(sys.getFieldVariable<Real>(tid, moose_obj->getParam<NonlinearVariableName>("v"))),
    _vel_x(_vel_x_var.adSln()),
    _vel_y(_vel_y_var.adSln()),
    _vel_z(moose_obj->isParamValid("w")
               ? &sys.getFieldVariable<Real>(tid, moose_obj->getParam<NonlinearVariableName>("w"))
                      .adSln()
               : nullptr),
    _comp(moose_obj->getParam<unsigned short>("component")),
    _gamma(moose_obj->getParam<Real>("gamma")),
    _face_velocity(getFunctor<ADRealVectorValue>("face_velocity")),
    _hmax(0)
{
  if ((mesh.dimension() > 2) && !moose_obj->isParamValid("w"))
    moose_obj->paramError("w", "For 3D meshes, the z-velocity must be provided");
}

void
MassFluxPenaltyIPHDGAssemblyHelper::scalarFace()
{
  _hmax = _ip_current_side_elem->hmax();

  for (const auto qp : make_range(_ip_qrule_face->n_points()))
  {
    const auto qp_term = computeQpResidualOnSide(qp) * _ip_JxW_face[qp];
    for (const auto i : index_range(_scalar_re))
      _scalar_re(i) += qp_term * _scalar_phi_face[i][qp];
  }
}

void
MassFluxPenaltyIPHDGAssemblyHelper::lmFace()
{
  _hmax = _ip_current_side_elem->hmax();

  for (const auto qp : make_range(_ip_qrule_face->n_points()))
  {
    const auto qp_term = computeQpResidualOnSide(qp) * _ip_JxW_face[qp];
    for (const auto i : index_range(_lm_re))
      _lm_re(i) -= qp_term * _lm_phi_face[i][qp];
  }
}

ADReal
MassFluxPenaltyIPHDGAssemblyHelper::computeQpResidualOnSide(const unsigned int qp)
{
  ADRealVectorValue soln_jump(_vel_x[qp], _vel_y[qp], 0);
  if (_vel_z)
    soln_jump(2) = (*_vel_z)[qp];
  soln_jump -= _face_velocity(
      Moose::ElemSideQpArg{
          _ip_current_elem, _ip_current_side, qp, _ip_qrule_face, _ip_q_point_face[qp]},
      _ti.determineState());

  return _gamma / _hmax * soln_jump * _ip_normals[qp] * _ip_normals[qp](_comp);
}
