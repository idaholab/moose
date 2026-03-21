//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MultiAppGeneralFieldKDTreeTransferBase.h"
#include "KDTree.h"
#include "SolutionInvalidInterface.h"
#include "NonADFunctorInterface.h"

/**
 * Transfers a functor (can be variable, function, functor material property, spatial UO, or PP)
 * Inside the domain of definition: evaluates the functor with the specified argument
 * Outside the domain of definition: uses a user-specified extrapolation behavior
 */
class MultiAppGeneralFieldFunctorTransfer : public MultiAppGeneralFieldKDTreeTransferBase,
                                            public NonADFunctorInterface

{
public:
  static InputParameters validParams();

  MultiAppGeneralFieldFunctorTransfer(const InputParameters & parameters);

  void initialSetup() override;
  void execute() override;

protected:
  virtual void prepareEvaluationOfInterpValues(const unsigned int var_index) override;

  // TODO: rename! We are not just interpolating anymore
  virtual void
  evaluateInterpValues(const unsigned int var_index,
                       const std::vector<std::pair<Point, unsigned int>> & incoming_points,
                       std::vector<std::pair<Real, Real>> & outgoing_vals) override;

private:
  /*
   * Build KD-Trees for each local app for the external values to extrapolate with
   * @param var_index the index of the variable being transferred
   * @details fills _local_kdtrees, _local_points and _local_values
   *          Indexing is: local apps (outer-indexing) OR positions (if using nearest_positions),
   *          local nodes (inner-indexing)
   */
  void buildKDTrees(const unsigned int var_index) override;

  /*
   * Evaluate values (interpolation and extrapolation) for incoming points
   * @param incoming_points all the points at which we need values
   * @param outgoing_vals vector containing the values and distances from point to nearest node
   */
  void evaluateValues(const unsigned int var_index,
                      const std::vector<std::pair<Point, unsigned int>> & incoming_points,
                      std::vector<std::pair<Real, Real>> & outgoing_vals);

  // Point locators for all source meshes
  std::vector<std::unique_ptr<libMesh::PointLocatorBase>> _point_locators;

  /// Names of the source functors
  const std::vector<MooseFunctorName> _functor_names;
  /// Pointers to the source functors
  std::vector<std::vector<const Moose::Functor<Real> *>> _functors;
  /// Whether the functor is a variable
  std::vector<bool> _functor_is_variable;

  /// How to determine values where the target mesh does not overlap the source mesh
  const MooseEnum _extrapolation_behavior;
};
