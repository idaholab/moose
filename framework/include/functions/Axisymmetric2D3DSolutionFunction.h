/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef AXISYMMETRIC2D3DSOLUTIONFUNCTION_H
#define AXISYMMETRIC2D3DSOLUTIONFUNCTION_H

#include "Function.h"

// Forward decleration
class Axisymmetric2D3DSolutionFunction;
class SolutionUserObject;

template <>
InputParameters validParams<Axisymmetric2D3DSolutionFunction>();

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
  Axisymmetric2D3DSolutionFunction(const InputParameters & parameters);

  virtual Real value(Real t, const Point & p) override;

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

#endif // AXISYMMETRIC2D3DSOLUTIONFUNCTION_H
