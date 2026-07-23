//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdvectionLHDGAssemblyHelper.h"

InputParameters
AdvectionLHDGAssemblyHelper::validParams()
{
  auto params = AdvectionHDGAssemblyHelper::validParams();
  params.addRequiredParam<MooseFunctorName>("face_velocity",
                                            "Hybrid velocity used for numerical face fluxes");
  return params;
}

AdvectionLHDGAssemblyHelper::AdvectionLHDGAssemblyHelper(
    const MooseObject * const moose_obj,
    MooseVariableDependencyInterface * const mvdi,
    const TransientInterface * const ti,
    SystemBase & sys,
    const Assembly & assembly,
    const THREAD_ID tid,
    const std::set<SubdomainID> & block_ids,
    const std::set<BoundaryID> & boundary_ids)
  : AdvectionHDGAssemblyHelper(moose_obj, mvdi, ti, sys, assembly, tid, block_ids, boundary_ids),
    ADFunctorInterface(moose_obj),
    _hybrid_velocity(getFunctor<ADRealVectorValue>("face_velocity"))
{
}

ADRealVectorValue
AdvectionLHDGAssemblyHelper::faceVelocity(const unsigned int qp) const
{
  return _hybrid_velocity(
      Moose::ElemSideQpArg{
          _current_elem, _current_side, qp, _qrule_face, _q_point_face[qp]},
      _ti.determineState());
}

void
AdvectionLHDGAssemblyHelper::lmDirichletZero()
{
  for (const auto qp : make_range(_qrule_face->n_points()))
    for (const auto i : index_range(_lm_re))
      // Match the zero-trace convention used by the L-HDG diffusion Dirichlet boundary condition.
      _lm_re(i) -= _JxW_face[qp] * _lm_u_sol[qp] * _lm_phi_face[i][qp];
}
