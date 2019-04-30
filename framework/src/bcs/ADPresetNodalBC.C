//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADPresetNodalBC.h"

// MOOSE includes
#include "MooseVariableFE.h"

#include "libmesh/numeric_vector.h"

defineADValidParams(ADPresetNodalBC,
                    ADNodalBC,
                    params.addClassDescription(
                        "Nodal boundary condition base class with preset solution vector values."));

template <ComputeStage compute_stage>
ADPresetNodalBC<compute_stage>::ADPresetNodalBC(const InputParameters & parameters)
  : ADNodalBC<compute_stage>(parameters)
{
}

template <ComputeStage compute_stage>
void
ADPresetNodalBC<compute_stage>::computeValue(NumericVector<Number> & current_solution)
{
  dof_id_type & dof_idx = _var.nodalDofIndex();
  current_solution.set(dof_idx, MetaPhysicL::raw_value(computeQpValue()));
}

template <ComputeStage compute_stage>
ADResidual
ADPresetNodalBC<compute_stage>::computeQpResidual()
{
  return _u - computeQpValue();
}

// explicit instantiation is required for AD base classes
adBaseClass(ADPresetNodalBC);
