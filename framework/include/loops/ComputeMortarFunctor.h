//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "MortarExecutorInterface.h"

#include "libmesh/libmesh_common.h"

class MortarConstraintBase;
class SubProblem;
class FEProblemBase;
class AutomaticMortarGeneration;
class Assembly;
class MooseMesh;
class MaterialBase;

namespace libMesh
{
class QBase;
template <typename>
class FEGenericBase;
typedef FEGenericBase<Real> FEBase;
}

class ComputeMortarFunctor : public MortarExecutorInterface
{
public:
  ComputeMortarFunctor(
      const std::vector<std::shared_ptr<MortarConstraintBase>> & mortar_constraints,
      const AutomaticMortarGeneration & amg,
      SubProblem & subproblem,
      FEProblemBase & fe_problem,
      bool displaced,
      Assembly & assembly);

  /**
   * Loops over the mortar segment mesh and computes the residual/Jacobian
   */
  void operator()(Moose::ComputeType compute_type,
                  const std::set<TagID> & vector_tag_ids,
                  const std::set<TagID> & matrix_tag_ids);

private:
  /// The mortar constraints to loop over when on each element. These must be
  /// pointers to the base class otherwise the compiler will fail to compile
  /// when running std::vector<MortarConstraint0>::push_back(MortarConstraint1>
  /// or visa versa
  std::vector<MortarConstraintBase *> _mortar_constraints;

  /// Automatic mortar generation (amg) object providing the mortar mesh to loop over
  const AutomaticMortarGeneration & _amg;

  /// A reference to the SubProblem object for reiniting lower-dimensional element quantities
  SubProblem & _subproblem;

  /// A reference to the FEProblemBase object for reiniting higher-dimensional element and neighbor
  /// element quantities. We use the FEProblemBase object for reiniting these because we may be
  /// using material properties from either undisplaced or displaced materials
  FEProblemBase & _fe_problem;

  /// Whether the mortar constraints are operating on the displaced mesh
  const bool _displaced;

  /// A reference to the assembly object
  Assembly & _assembly;
};
