//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdvectionIPHDGAssemblyHelper.h"
#include "MooseTypes.h"
#include "MooseVariableDependencyInterface.h"
#include "MooseVariableScalar.h"
#include "SystemBase.h"
#include "MooseMesh.h"
#include "MooseObject.h"
#include "MaterialPropertyInterface.h"

using namespace libMesh;

InputParameters
AdvectionIPHDGAssemblyHelper::validParams()
{
  auto params = IPHDGAssemblyHelper::validParams();
  params.addRequiredParam<MaterialPropertyName>("velocity", "Velocity vector");
  params.addParam<Real>(
      "coeff", 1, "A constant coefficient. This could be something like a density");
  params.addParam<bool>("self_advection",
                        true,
                        "Whether this kernel should advect itself, e.g. it's "
                        "variable/side_variable pair. If false, we will advect "
                        "unity (possibly multiplied by the 'coeff' parameter");
  return params;
}

AdvectionIPHDGAssemblyHelper::AdvectionIPHDGAssemblyHelper(
    const MooseObject * const moose_obj,
    MooseVariableDependencyInterface * const mvdi,
    const TransientInterface * const ti,
    SystemBase & sys,
    const Assembly & assembly,
    const THREAD_ID tid,
    const std::set<SubdomainID> & block_ids,
    const std::set<BoundaryID> & boundary_ids)
  : IPHDGAssemblyHelper(moose_obj, mvdi, ti, sys, assembly, tid, block_ids, boundary_ids),
    _velocity(getADMaterialProperty<RealVectorValue>("velocity")),
    _face_velocity(getFaceADMaterialProperty<RealVectorValue>("velocity")),
    _coeff(moose_obj->getParam<Real>("coeff")),
    _self_advection(moose_obj->getParam<bool>("self_advection"))
{
}

void
AdvectionIPHDGAssemblyHelper::scalarVolume()
{
  for (const auto qp : make_range(_ip_qrule->n_points()))
  {
    ADReal adv_quant = _coeff;
    if (_self_advection)
      adv_quant *= _u_sol[qp];
    for (const auto i : index_range(_scalar_re))
      _scalar_re(i) -= _ip_JxW[qp] * _grad_scalar_phi[i][qp] * _velocity[qp] * adv_quant;
  }
}

void
AdvectionIPHDGAssemblyHelper::scalarFace()
{
  for (const auto qp : make_range(_ip_qrule_face->n_points()))
  {
    const auto vdotn = _face_velocity[qp] * _ip_normals[qp];
    ADReal adv_quant = _coeff;
    if (_self_advection)
      adv_quant *= (MetaPhysicL::raw_value(vdotn) >= 0 ? _u_sol[qp] : _lm_u_sol[qp]);
    for (const auto i : index_range(_scalar_re))
      _scalar_re(i) += _ip_JxW_face[qp] * _scalar_phi_face[i][qp] * vdotn * adv_quant;
  }
}

void
AdvectionIPHDGAssemblyHelper::lmFace()
{
  for (const auto qp : make_range(_ip_qrule_face->n_points()))
  {
    const auto vdotn = _face_velocity[qp] * _ip_normals[qp];
    ADReal adv_quant = _coeff;
    if (_self_advection)
      adv_quant *= (MetaPhysicL::raw_value(vdotn) >= 0 ? _u_sol[qp] : _lm_u_sol[qp]);
    for (const auto i : index_range(_lm_re))
      _lm_re(i) -= _ip_JxW_face[qp] * _lm_phi_face[i][qp] * vdotn * adv_quant;
  }
}

void
AdvectionIPHDGAssemblyHelper::scalarDirichlet(const Moose::Functor<Real> & dirichlet_functor)
{
  for (const auto qp : make_range(_ip_qrule_face->n_points()))
  {
    const auto vdotn = _face_velocity[qp] * _ip_normals[qp];
    mooseAssert(_self_advection, "This shouldn't be called if we are not self-advecting");
    const auto dirichlet_value = dirichlet_functor(
        Moose::ElemSideQpArg{
            _ip_current_elem, _ip_current_side, qp, _ip_qrule_face, _ip_q_point_face[qp]},
        _ti.determineState());
    const auto adv_quant = dirichlet_value * _coeff;
    for (const auto i : index_range(_scalar_re))
      _scalar_re(i) += _ip_JxW_face[qp] * _scalar_phi_face[i][qp] * vdotn * adv_quant;
  }
}

void
AdvectionIPHDGAssemblyHelper::lmOutflow()
{
  for (const auto qp : make_range(_ip_qrule_face->n_points()))
  {
#ifndef NDEBUG
    const auto vdotn = _face_velocity[qp] * _ip_normals[qp];
    mooseAssert(MetaPhysicL::raw_value(vdotn) >= 0, "The velocity must create outflow conditions");
    mooseAssert(_self_advection, "This shouldn't be called if we are not self-advecting");
#endif
    for (const auto i : index_range(_lm_re))
      // Force the LM solution to be equivalent to the internal solution
      _lm_re(i) += _ip_JxW_face[qp] * _coeff * (_lm_u_sol[qp] - _u_sol[qp]) * _lm_phi_face[i][qp];
  }
}
