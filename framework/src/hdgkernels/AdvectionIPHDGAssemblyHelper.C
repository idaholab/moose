//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdvectionIPHDGAssemblyHelper.h"
InputParameters
AdvectionIPHDGAssemblyHelper::validParams()
{
  return AdvectionHDGAssemblyHelper::validParams();
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
  : AdvectionHDGAssemblyHelper(moose_obj, mvdi, ti, sys, assembly, tid, block_ids, boundary_ids),
    _face_velocity(getFaceADMaterialProperty<RealVectorValue>("velocity"))
{
}

ADRealVectorValue
AdvectionIPHDGAssemblyHelper::faceVelocity(const unsigned int qp) const
{
  return _face_velocity[qp];
}
