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

#ifndef POINTSAMPLERBASE_H
#define POINTSAMPLERBASE_H

// MOOSE includes
#include "GeneralVectorPostprocessor.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"
#include "SamplerBase.h"

// Forward Declarations
class PointSamplerBase;

template <>
InputParameters validParams<PointSamplerBase>();

class PointSamplerBase : public GeneralVectorPostprocessor,
                         public CoupleableMooseVariableDependencyIntermediateInterface,
                         protected SamplerBase
{
public:
  PointSamplerBase(const InputParameters & parameters);

  virtual ~PointSamplerBase() {}

  virtual void initialize();
  virtual void execute();
  virtual void finalize();

protected:
  /**
   * Find the local element that contains the point.  This will attempt to use a cached element to
   * speed things up.
   *
   * @param p The point in physical space
   * @return The Elem containing the point or NULL if this processor doesn't contain an element that
   * contains this point.
   */
  const Elem * getLocalElemContainingPoint(const Point & p);

  /// The Mesh we're using
  MooseMesh & _mesh;

  /// The points to evaluate at
  std::vector<Point> _points;

  /// The ID to use for each point (yes, this is Real on purpose)
  std::vector<Real> _ids;

  /// Vector of values per point
  std::vector<std::vector<Real>> _point_values;

  /// Whether or not the Point was found on this processor (short because bool and char don't work with MPI wrappers)
  std::vector<short> _found_points;

  unsigned int _qp;

  std::unique_ptr<PointLocatorBase> _pl;
};

#endif
