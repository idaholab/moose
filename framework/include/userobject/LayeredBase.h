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

#ifndef LAYEREDBASE_H
#define LAYEREDBASE_H

#include "InputParameters.h"
#include "UserObject.h"

// libmesh includes
#include "libmesh/mesh_tools.h"

//Forward Declarations
class LayeredBase;

template<>
InputParameters validParams<LayeredBase>();

/**
 * This UserObject computes volume integrals of a variable storing partial sums for the specified number of intervals in a direction (x,y,z).c
 */
class LayeredBase
{
public:
  LayeredBase(const std::string & name, InputParameters parameters);

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

  /// Name of this object
  std::string _layered_base_name;

  /// Params for this object
  InputParameters _layered_base_params;

  /// The MooseEnum direction the layers are going in
  MooseEnum _direction_enum;

  /// The component direction the layers are going in.  We cache this for speed (so we're not always going through the MooseEnum)
  unsigned int _direction;

  /// Number of layers to split the mesh into
  unsigned int _num_layers;

  /// How to sample the values
  unsigned int _sample_type;

  /// How many layers both above and below the found layer will be used in the average
  unsigned int _average_radius;

  Real _direction_min;
  Real _direction_max;

private:
  /// Value of the integral for each layer
  std::vector<Real> _layer_values;

  /// Whether or not each layer has had any value summed into it
  std::vector<bool> _layer_has_value;

  /// Subproblem for the child object
  SubProblem & _layered_base_subproblem;
};

#endif
