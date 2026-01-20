//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdvectionIPHDGAssemblyHelper.h"
#include "MooseTypes.h"
#include "MooseObject.h"
#include "MaterialPropertyInterface.h"

using namespace libMesh;

InputParameters
AdvectionIPHDGAssemblyHelper::validParams()
{
  auto params = IPHDGAssemblyHelper::validParams();
  params.addRequiredParam<MaterialPropertyName>("velocity", "Velocity vector");
  params.addRequiredParam<Real>("coeff",
                                "A constant coefficient. This could be something like a density");
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
    _coeff(moose_obj->getParam<Real>("coeff"))
{
}

void
AdvectionIPHDGAssemblyHelper::scalarVolume()
{
  for (const auto qp : make_range(_ip_qrule->n_points()))
  {
    const auto adv_quant = _coeff * _u_sol[qp];
    const auto qp_term = _ip_JxW[qp] * _velocity[qp] * adv_quant;
    for (const auto i : index_range(_scalar_re))
      _scalar_re(i) -= _grad_scalar_phi[i][qp] * qp_term;
  }
}

ADReal
AdvectionIPHDGAssemblyHelper::computeFlux(const unsigned int qp, const ADReal & face_value)
{
  const auto vdotn = _face_velocity[qp] * _ip_normals[qp];
  const auto face_phi = _coeff * face_value;
  const auto internal_phi = _coeff * _u_sol[qp];
  // Short form for writing upwinding. If the velocity is in the direction of the surface normal,
  // the the internal value is used, else the face value is used
  return 0.5 * vdotn * (internal_phi + face_phi) + 0.5 * abs(vdotn) * (internal_phi - face_phi);
}

void
AdvectionIPHDGAssemblyHelper::scalarFace()
{
  for (const auto qp : make_range(_ip_qrule_face->n_points()))
  {
    const auto qp_term = _ip_JxW_face[qp] * computeFlux(qp, _lm_u_sol[qp]);
    for (const auto i : index_range(_scalar_re))
      _scalar_re(i) += _scalar_phi_face[i][qp] * qp_term;
  }
}

void
AdvectionIPHDGAssemblyHelper::lmFace()
{
  for (const auto qp : make_range(_ip_qrule_face->n_points()))
  {
    const auto qp_term = _ip_JxW_face[qp] * computeFlux(qp, _lm_u_sol[qp]);
    for (const auto i : index_range(_lm_re))
      _lm_re(i) -= _lm_phi_face[i][qp] * qp_term;
  }
}

void
AdvectionIPHDGAssemblyHelper::scalarDirichlet(const Moose::Functor<Real> & dirichlet_functor)
{
  for (const auto qp : make_range(_ip_qrule_face->n_points()))
  {
    const auto dirichlet_value = dirichlet_functor(
        Moose::ElemSideQpArg{
            _ip_current_elem, _ip_current_side, qp, _ip_qrule_face, _ip_q_point_face[qp]},
        _ti.determineState());
    const auto qp_term = _ip_JxW_face[qp] * computeFlux(qp, dirichlet_value);
    for (const auto i : index_range(_scalar_re))
      _scalar_re(i) += _scalar_phi_face[i][qp] * qp_term;
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
#endif
    const auto qp_term = _ip_JxW_face[qp] * _coeff * (_lm_u_sol[qp] - _u_sol[qp]);
    for (const auto i : index_range(_lm_re))
      // Force the LM solution to be equivalent to the internal solution
      _lm_re(i) += _lm_phi_face[i][qp] * qp_term;
  }
}
