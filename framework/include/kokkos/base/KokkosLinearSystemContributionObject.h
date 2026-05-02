//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosDispatcher.h"
#include "KokkosMesh.h"
#include "KokkosSystem.h"
#include "KokkosVariable.h"

#include "MooseObject.h"
#include "SetupInterface.h"
#include "FunctionInterface.h"
#include "UserObjectInterface.h"
#include "TransientInterface.h"
#include "PostprocessorInterface.h"
#include "VectorPostprocessorInterface.h"
#include "Restartable.h"
#include "MeshChangedInterface.h"
#include "TaggingInterface.h"
#include "LinearSystemContributionObject.h"

class InputParameters;
class FEProblemBase;

namespace Moose::Kokkos
{

class LinearSystemContributionObject : public MooseObject,
                                       public SetupInterface,
                                       public FunctionInterface,
                                       public UserObjectInterface,
                                       public TransientInterface,
                                       public PostprocessorInterface,
                                       public VectorPostprocessorInterface,
                                       public Restartable,
                                       public MeshChangedInterface,
                                       public TaggingInterface,
                                       public MeshHolder,
                                       public SystemHolder
{
public:
  static InputParameters validParams();

  LinearSystemContributionObject(const InputParameters & parameters);
  LinearSystemContributionObject(const LinearSystemContributionObject & object);

protected:
  KOKKOS_FUNCTION void accumulateTaggedVector(Real value, dof_id_type row) const;
  KOKKOS_FUNCTION void accumulateTaggedMatrix(Real value, dof_id_type row, dof_id_type col) const;

protected:
  FEProblemBase & _fe_problem;

  /**
   * Kokkos variable
   */
  Variable _kokkos_var;

  Array<TagID> _vector_tags;
  Array<TagID> _matrix_tags;

  Scalar<Real> _t;
  Scalar<const Real> _t_old;
  Scalar<int> _t_step;
  Scalar<Real> _dt;
  Scalar<Real> _dt_old;
};

KOKKOS_FUNCTION inline void
LinearSystemContributionObject::accumulateTaggedVector(const Real value,
                                                       const dof_id_type row) const
{
  if (!value)
    return;

  KOKKOS_ASSERT(_kokkos_var.components() == 1);
  auto & sys = kokkosSystem(_kokkos_var.sys());
  for (dof_id_type index = 0; index < _vector_tags.size(); ++index)
  {
    const auto tag = _vector_tags[index];
    if (sys.isResidualTagActive(tag))
      ::Kokkos::atomic_add(&sys.getVectorDofValue(row, tag), value);
  }
}

KOKKOS_FUNCTION inline void
LinearSystemContributionObject::accumulateTaggedMatrix(const Real value,
                                                       const dof_id_type row,
                                                       const dof_id_type col) const
{
  if (!value)
    return;

  KOKKOS_ASSERT(_kokkos_var.components() == 1);
  auto & sys = kokkosSystem(_kokkos_var.sys());
  for (dof_id_type index = 0; index < _matrix_tags.size(); ++index)
  {
    const auto tag = _matrix_tags[index];
    if (sys.isMatrixTagActive(tag))
      ::Kokkos::atomic_add(&sys.getMatrixValue(row, col, tag), value);
  }
}

} // namespace Moose::Kokkos
