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

#ifndef LAYEREDINTEGRAL_H
#define LAYEREDINTEGRAL_H

#include "ElementIntegralVariableUserObject.h"

// libmesh includes
#include "libmesh/mesh_tools.h"

//Forward Declarations
class LayeredIntegral;

template<>
InputParameters validParams<LayeredIntegral>();

/**
 * This UserObject computes volume integrals of a variable storing partial sums for the specified number of intervals in a direction (x,y,z).c
 */
class LayeredIntegral : public ElementIntegralVariableUserObject
{
public:
  LayeredIntegral(const std::string & name, InputParameters parameters);

  /**
   * Given a Point return the integral value associated with the layer that point falls in.
   *
   * @param p The point to look for in the layers.
   */
  Real integralValue(Point p) const;

  /**
   * Given a Point return the integral value associated with the layer that point falls in.
   *
   * @param p The point to look for in the layers.
   */
  virtual Real spatialValue(const Point & p) const { return integralValue(p); }

  /**
   * Get the value for a given layer
   * @param layer The layer index
   * @return The value for the given layer
   */
  virtual Real getLayerValue(unsigned int layer) const;

  virtual void initialize();
  virtual void execute();
  virtual void finalize();
  virtual void threadJoin(const UserObject & y);

protected:
  /**
   * Helper function to return the layer the point lies in.
   * @param p The point.
   * @return The layer the Point is found in.
   */
  unsigned int getLayer(Point p) const;

  /// The MooseEnum direction the layers are going in
  MooseEnum _direction_enum;

  /// The component direction the layers are going in.  We cache this for speed (so we're not always going through the MooseEnum)
  unsigned int _direction;

  /// Number of layers to split the mesh into
  unsigned int _num_layers;

  /// Value of the integral for each layer
  std::vector<Real> _layer_values;

  Real _direction_min;
  Real _direction_max;
};

#endif
