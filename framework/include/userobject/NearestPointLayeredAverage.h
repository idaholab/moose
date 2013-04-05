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

#ifndef NEARESTPOINTLAYEREDAVERAGE_H
#define NEARESTPOINTLAYEREDAVERAGE_H

#include "ElementIntegralVariableUserObject.h"
#include "LayeredAverage.h"

// libmesh includes
#include "libmesh/mesh_tools.h"

//Forward Declarations
class NearestPointLayeredAverage;

template<>
InputParameters validParams<NearestPointLayeredAverage>();

/**
 * This UserObject computes  averages of a variable storing partial sums for the specified number of intervals in a direction (x,y,z).
 *
 * Given a list of points this object computes the layered average closest to each one of those points.
 */
class NearestPointLayeredAverage : public ElementIntegralVariableUserObject
{
public:
  NearestPointLayeredAverage(const std::string & name, InputParameters parameters);
  ~NearestPointLayeredAverage();

  virtual void initialize();
  virtual void execute();
  virtual void finalize();
  virtual void threadJoin(const UserObject & y);

  /**
   * Given a Point return the integral value associated with the layer that point falls in for the layered average closest to that point.
   *
   * @param p The point to look for in the layers.
   */
  virtual Real spatialValue(const Point & p) const { return nearestLayeredAverage(p)->integralValue(p); }

protected:
  /**
   * Get the LayeredAverage that is closest to the point.
   *
   * @param p The point.
   * @return The LayeredAverage closest to p.
   */
  LayeredAverage * nearestLayeredAverage(const Point & p) const;

  std::vector<Point> _points;
  std::vector<LayeredAverage *> _layered_averages;
};

#endif
