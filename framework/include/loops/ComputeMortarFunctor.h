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

#include "libmesh/libmesh_common.h"

template <ComputeStage>
class MortarConstraint;
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
  // /**
  //  * Computes a residual
  //  */
  // void computeElementResidual();

  // /**
  //  * Computes a jacobian
  //  */
  // void computeElementJacobian();

private:
  /// The mortar constraints to loop over when on each element. These must be
  /// pointers to the base class otherwise the compiler will fail to compile
  /// when running std::vector<MortarConstraint0>::push_back(MortarConstraint1>
  /// or visa versa
  std::vector<const MortarConstraintBase *> _mortar_constraints;

  /// Automatic mortar generation (amg) object providing the mortar mesh to loop over
  const AutomaticMortarGeneration & _amg;

  /// A reference to the SubProblem object for reiniting
  SubProblem & _subproblem;

  /// The assembly object that these constraints will contribute to
  Assembly & _assembly;

  /// The parent mesh from which the amg mortar mesh is generated
  MooseMesh & _moose_parent_mesh;

  /// The interior mesh dimension
  const unsigned _interior_dimension;

  /// The mortar segment mesh dimension
  const unsigned _msm_dimension;

  /// The mortar segment quadrature rule
  libMesh::QBase * const & _qrule_msm;

  /// The FE object used for generating JxW
  std::unique_ptr<libMesh::FEBase> _fe_for_jxw;

  /// Pointer to the mortar segment JxW
  const std::vector<libMesh::Real> * _JxW_msm;

  /// The slave boundary id needed for reiniting the MOOSE systems on the element face
  BoundaryID _slave_boundary_id;

  /// boolean flag for holding whether our current mortar segment projects onto a master element
  bool _has_master;
};

#endif // COMPUTEMORTARFUNCTOR_H
