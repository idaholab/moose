//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Function.h"

class SolutionUserObject;

/**
 * Function for reading a 2D axisymmetric solution from file and mapping it to a
 * 3D Cartesian system. This function extracts values from a solution read from a
 * file via a SolutionUserObject. The appropriate transformations are applied to
 * convert either scalar or vector functions from a 2D axisymmetric frame to a
 * 3D Cartesian frame. It is possible to scale and add a constant to the solution.
 */
class Axisymmetric2D3DSolutionFunction : public Function
{
public:
  /**
   * Constructor
   * @param parameters The input parameters for the function
   */
  static InputParameters validParams();

  Axisymmetric2D3DSolutionFunction(const InputParameters & parameters);

  using Function::value;
  virtual Real value(Real t, const Point & p) const override;

  /**
   * Setup the function for use
   * Gathers a pointer to the SolutionUserObject containing the solution that
   * was read. A pointer is required because Functions are created prior to UserObjects,
   * see Moose.C.
   */
  virtual void initialSetup() override;

protected:
  /// Pointer to SolutionUserObject containing the solution of interest
  const SolutionUserObject * _solution_object_ptr;

  /// Factor to scale the solution by (default = 1)
  const Real _scale_factor;

  /// Factor to add to the solution (default = 0)
  const Real _add_factor;

  /// Ratio of axial dimension of 3d model to its counterpart in the 2d model.
  /// This permits the axial dimension of the 3d model to be larger than that
  /// of the 2d model
  const Real _axial_dim_ratio;

  /// Two points that define the axis of rotation for the 2d model
  const RealVectorValue _2d_axis_point1;
  const RealVectorValue _2d_axis_point2;

  /// Two points that define the axis of rotation for the 3d model
  const RealVectorValue _3d_axis_point1;
  const RealVectorValue _3d_axis_point2;

  /// If the solution field is a vector, the desired component must be specified
  /// Has the component been specified?
  const bool _has_component;

  /// The index of the component
  const unsigned int _component;

  /// Are the default axes of rotation being used?
  bool _default_axes;

  /// The variable names to extract from the file
  std::vector<std::string> _var_names;

  /// The local SolutionUserObject indices for the variables extracted from the file
  std::vector<unsigned int> _solution_object_var_indices;
};
