//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTEMORTARFUNCTOR_H
#define COMPUTEMORTARFUNCTOR_H

#include "MooseTypes.h"

#include "libmesh/libmesh_common.h"

class MortarConstraintBase;
class SubProblem;
class AutomaticMortarGeneration;
class Assembly;
class MooseMesh;

namespace libMesh
{
class QBase;
template <typename>
class FEGenericBase;
typedef FEGenericBase<Real> FEBase;
}

template <ComputeStage compute_stage>
class ComputeMortarFunctor
{
public:
  ComputeMortarFunctor(
      const std::vector<std::shared_ptr<MortarConstraintBase>> & mortar_constraints,
      const AutomaticMortarGeneration & amg,
      SubProblem & subproblem);

  /**
   * Loops over the mortar segment mesh and computes the residual/Jacobian
   */
  void operator()();

private:
  /// The mortar constraints to loop over when on each element. These must be
  /// pointers to the base class otherwise the compiler will fail to compile
  /// when running std::vector<MortarConstraint0>::push_back(MortarConstraint1>
  /// or visa versa
  std::vector<MortarConstraintBase *> _mortar_constraints;

  /// Automatic mortar generation (amg) object providing the mortar mesh to loop over
  const AutomaticMortarGeneration & _amg;

  /// A reference to the SubProblem object for reiniting
  SubProblem & _subproblem;

  /// A reference to the assembly object
  Assembly & _assembly;

  /// The mortar quadrature rule. Necessary for sizing the number of custom
  /// points for re-init'ing the slave interior, master interior, and slave face
  /// elements
  const libMesh::QBase * const & _qrule_msm;

  /// The slave boundary id needed for reiniting the MOOSE systems on the element (slave) face
  BoundaryID _slave_boundary_id;

  /// The master boundary id needed for reiniting the MOOSE systems on the neighbor (master) face
  BoundaryID _master_boundary_id;

  /// boolean flag for holding whether our current mortar segment projects onto a master element
  bool _has_master;
};

#endif // COMPUTEMORTARFUNCTOR_H
