//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Moose.h"
#include "MooseEnum.h"
#include "MooseTypes.h"
#include "Restartable.h"

// Forward Declarations
class InputParameters;
class SubProblem;
class UserObject;

namespace libMesh
{
class Point;
}

template <typename T>
InputParameters validParams();

/**
 * This base class computes volume integrals of a variable storing
 * partial sums for the specified number of intervals in a direction
 * (x,y,z).
 */
class LayeredBase : private Restartable
{
public:
  static InputParameters validParams();

  LayeredBase(const InputParameters & parameters);

  /**
   * Given a Point return the integral value associated with the layer that point falls in.
   *
   * @param p The point to look for in the layers.
   */
  virtual Real integralValue(Point p) const;

  /**
   * Get the value for a given layer
   * @param layer The layer index
   * @return The value for the given layer
   */
  virtual Real getLayerValue(unsigned int layer) const;

  /**
   * Helper function to return the layer the point lies in.
   * @param p The point.
   * @return The layer the Point is found in.
   */
  virtual unsigned int getLayer(Point p) const;

  /**
   * Get the center coordinates for the layers (along given direction)
   */
  const std::vector<Real> & getLayerCenters() const { return _layer_centers; }

  /**
   * Get direction of the layers
   * @return layer direction
   */
  unsigned int direction() const { return _direction; }

  virtual void initialize();
  virtual void finalize();
  virtual void threadJoin(const UserObject & y);

protected:
  /**
   * Set the value for a particular layer
   * @param layer The layer you are setting the value for
   * @param value The value to set
   */
  void setLayerValue(unsigned int layer, Real value);

  /**
   * Whether or not a layer has a value.
   */
  bool layerHasValue(unsigned int layer) const { return _layer_has_value[layer]; }

  /**
   * Compute bounds, restricted to blocks if given
   */
  void getBounds();

  /**
   * Compute the center points for each layer
   */
  void computeLayerCenters();

  /// Name of this object
  std::string _layered_base_name;

  /// Params for this object
  const InputParameters & _layered_base_params;

  /// The MooseEnum direction the layers are going in
  MooseEnum _direction_enum;

  /// The component direction the layers are going in.  We cache this for speed (so we're not always going through the MooseEnum)
  unsigned int _direction;

  /// Whether or not this object is based on equally spaced intervals or "bounds"
  bool _interval_based;

  /// Number of layers to split the mesh into
  unsigned int _num_layers;

  /// The boundaries of the layers
  std::vector<Real> _layer_bounds;

  /// How to sample the values
  unsigned int _sample_type;

  /// How many layers both above and below the found layer will be used in the average
  unsigned int _average_radius;

  /// true if this object operates on the displaced mesh, otherwise false
  bool _using_displaced_mesh;

  /// center coordinates of each layer
  std::vector<Real> _layer_centers;

  Real _direction_min;
  Real _direction_max;

private:
  /// Value of the integral for each layer
  std::vector<Real> & _layer_values;

  /// Whether or not each layer has had any value summed into it
  std::vector<int> & _layer_has_value;

  /// Subproblem for the child object
  SubProblem & _layered_base_subproblem;

  /// Whether the values are cumulative over the layers
  bool _cumulative;

  /// Whether the cumulative values should be summed in the positive or negative direction
  const bool _positive_cumulative_direction;

  /// List of SubdomainIDs, if given
  std::vector<SubdomainID> _layer_bounding_blocks;

  /// whether the max/min coordinate in the direction is known from user input
  bool _has_direction_max_min;
};
