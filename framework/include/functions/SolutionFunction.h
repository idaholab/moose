//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Function.h"
#include "SolutionUserObjectBase.h"

/** Function for reading a solution from file
 * Creates a function that extracts values from a solution read from a file,
 * via a SolutionUserObject. It is possible to scale and add a constant to the
 * solution read.
 */
class SolutionFunction : public Function
{
public:
  /** Constructor
   * @param parameters The input parameters for the function
   */
  static InputParameters validParams();

  SolutionFunction(const InputParameters & parameters);

  using Function::value;
  /**
   * Extract a value from the solution
   * @param t Time at which to extract
   * @param p Spatial location of desired data
   * @return The value at t and p
   */
  virtual Real value(Real t, const Point & p) const override;

  /**
   * Extract a gradient from the solution
   * @param t Time at which to extract
   * @param p Spatial location of desired data
   * @return The value at t and p
   */
  virtual RealGradient gradient(Real t, const Point & p) const override;

  /**
   * Setup the function for use
   * Gathers a pointer to the SolutionUserObject containing the solution that
   * was read. A pointer is required because Functions are created prior to UserObjects,
   * see Moose.C.
   */
  virtual void initialSetup() override;

protected:
  /// Pointer to SolutionUserObject containing the solution of interest
  const SolutionUserObjectBase * _solution_object_ptr;

  /// The name of the variable extracted from the imported solution
  std::string _solution_object_var_name;

  /// Policy used when the imported solution is multivalued at the query point
  const SolutionUserObjectBase::WeightingType _weighting_type;

  /// Factor to scale the solution by (default = 1)
  const Real _scale_factor;

  /// Factor to add to the solution (default = 0)
  const Real _add_factor;

  /// Factor to add to the solution if gradient is requested (default = \vec{0})
  RealGradient _add_grad;
};
