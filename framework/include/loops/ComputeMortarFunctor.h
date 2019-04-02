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

template <ComputeStage compute_stage>
class ComputeMortarFunctor
{
public:
  ComputeMortarFunctor(std::vector<std::shared_ptr<MortarConstraintBase>> & mortar_constraints,
                       const AutomaticMortarGeneration & amg);

  /**
   * Loops over the mortar segment mesh and computes the residual/Jacobian
   */
  void operator()();

private:
  /// The mortar constraints to loop over when on each element
  std::vector<std::shared_ptr<MortarConstraint<compute_stage>>> _mortar_constraints;

  /// Automatic mortar generation (amg) object providing the mortar mesh to loop over
  const AutomaticMortarGeneration & _amg;

  /// A reference to the FEProblemBase object because who knows when you're going to need it
  FEProblemBase & _fe_problem;

  /// whether this functor is on the displaced mesh
  const bool _on_displaced;

  /// The assembly object that these constraints will contribute to
  Assembly & _assembly;

  /// The parent mesh from which the amg mortar mesh is generated
  MooseMesh & _moose_parent_mesh;

  /// The interior mesh dimension
  const unsigned _interior_dimension;

  /// The mortar segment mesh dimension
  const unsigned _msm_dimension;

  /// The mortar segment quadrature rule
  QBase *& _qrule_msm;

  /// The FE object used for generating JxW
  std::unique_ptr<FEBase> _fe_for_jxw;

  /// Pointer to the mortar segment JxW
  const std::vector<Real> * _JxW_msm;

  /// Container of variables for which values need to be computed
  std::set<MooseVariable *> _variables_needed_for_values;

  /// Container of variables for which gradients need to be computed
  std::set<MooseVariable *> _variables_needed_for_gradients;
};

#endif // COMPUTEMORTARFUNCTOR_H
