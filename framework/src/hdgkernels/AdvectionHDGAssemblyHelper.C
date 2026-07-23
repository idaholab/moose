//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdvectionHDGAssemblyHelper.h"
#include "IPHDGAssemblyHelper.h"
#include "MooseObject.h"

using namespace libMesh;

template <typename Base>
InputParameters
AdvectionHDGAssemblyHelperTempl<Base>::validParams()
{
  auto params = Base::validParams();
  params.template addRequiredParam<MaterialPropertyName>(
      "velocity", "The cell-interior velocity material property used in the volume advection term");
  params.template addRequiredParam<Real>(
      "coeff", "Constant coefficient multiplying the advected scalar, such as density");
  return params;
}

template <typename Base>
AdvectionHDGAssemblyHelperTempl<Base>::AdvectionHDGAssemblyHelperTempl(
    const MooseObject * const moose_obj,
    MooseVariableDependencyInterface * const mvdi,
    const TransientInterface * const ti,
    SystemBase & sys,
    const Assembly & assembly,
    const THREAD_ID tid,
    const std::set<SubdomainID> & block_ids,
    const std::set<BoundaryID> & boundary_ids)
  : Base(moose_obj, mvdi, ti, sys, assembly, tid, block_ids, boundary_ids),
    _velocity(this->template getADMaterialProperty<RealVectorValue>("velocity")),
    _coeff(moose_obj->getParam<Real>("coeff"))
{
}

template <typename Base>
void
AdvectionHDGAssemblyHelperTempl<Base>::scalarVolume()
{
  for (const auto qp : make_range(_qrule->n_points()))
  {
    const auto advected_quantity = _coeff * _u_sol[qp];
    const auto qp_term = _JxW[qp] * _velocity[qp] * advected_quantity;
    for (const auto i : index_range(_scalar_re))
      _scalar_re(i) -= _grad_scalar_phi[i][qp] * qp_term;
  }
}

template <typename Base>
ADReal
AdvectionHDGAssemblyHelperTempl<Base>::computeFlux(const unsigned int qp,
                                                   const ADReal & face_value) const
{
  const auto vdotn = faceVelocity(qp) * _normals[qp];
  const auto face_phi = _coeff * face_value;
  const auto internal_phi = _coeff * _u_sol[qp];
  // If velocity points out of the element, use the interior value; otherwise use the face value.
  return 0.5 * vdotn * (internal_phi + face_phi) + 0.5 * abs(vdotn) * (internal_phi - face_phi);
}

template <typename Base>
void
AdvectionHDGAssemblyHelperTempl<Base>::scalarFace()
{
  for (const auto qp : make_range(_qrule_face->n_points()))
  {
    const auto qp_term = _JxW_face[qp] * computeFlux(qp, _lm_u_sol[qp]);
    for (const auto i : index_range(_scalar_re))
      _scalar_re(i) += _scalar_phi_face[i][qp] * qp_term;
  }
}

template <typename Base>
void
AdvectionHDGAssemblyHelperTempl<Base>::lmFace()
{
  for (const auto qp : make_range(_qrule_face->n_points()))
  {
    const auto qp_term = _JxW_face[qp] * computeFlux(qp, _lm_u_sol[qp]);
    for (const auto i : index_range(_lm_re))
      _lm_re(i) -= _lm_phi_face[i][qp] * qp_term;
  }
}

template <typename Base>
void
AdvectionHDGAssemblyHelperTempl<Base>::scalarDirichlet(
    const Moose::Functor<Real> & dirichlet_functor)
{
  for (const auto qp : make_range(_qrule_face->n_points()))
  {
    const auto dirichlet_value = dirichlet_functor(
        Moose::ElemSideQpArg{
            _current_elem, _current_side, qp, _qrule_face, _q_point_face[qp]},
        _ti.determineState());
    const auto qp_term = _JxW_face[qp] * computeFlux(qp, dirichlet_value);
    for (const auto i : index_range(_scalar_re))
      _scalar_re(i) += _scalar_phi_face[i][qp] * qp_term;
  }
}

template <typename Base>
void
AdvectionHDGAssemblyHelperTempl<Base>::lmOutflow()
{
  for (const auto qp : make_range(_qrule_face->n_points()))
  {
#ifndef NDEBUG
    const auto vdotn = faceVelocity(qp) * _normals[qp];
    mooseAssert(MetaPhysicL::raw_value(vdotn) >= 0, "The velocity must create outflow conditions");
#endif
    const auto qp_term = _JxW_face[qp] * _coeff * (_lm_u_sol[qp] - _u_sol[qp]);
    for (const auto i : index_range(_lm_re))
      // Force the facet solution to be equivalent to the interior solution.
      _lm_re(i) += _lm_phi_face[i][qp] * qp_term;
  }
}

template class AdvectionHDGAssemblyHelperTempl<HDGAssemblyHelper>;
template class AdvectionHDGAssemblyHelperTempl<IPHDGAssemblyHelper>;
