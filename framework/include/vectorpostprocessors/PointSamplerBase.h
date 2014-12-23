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

#include "GeneralVectorPostprocessor.h"
#include "SamplerBase.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"

//Forward Declarations
class PointSamplerBase;

template<>
InputParameters validParams<PointSamplerBase>();

/**
 * Base class for point based VectorPostprocessors
 */
class PointSamplerBase :
  public GeneralVectorPostprocessor,
  public CoupleableMooseVariableDependencyIntermediateInterface,
  protected SamplerBase
{
public:
  /**
   * Constructor
   * @param name The name of the VectorPostprocessor
   * @param paramters Input parameters for this VectorPostprocessor
   */
  PointSamplerBase(const std::string & name, InputParameters parameters);

  /**
   * Destructor
   */
  virtual ~PointSamplerBase() {}

  /**
   * Initialization method
   * Calls SampleBase::initialize and populates the local point ids allowing for
   * execute() to mimize the number of point locator calls
   */
  virtual void initialize();

  /**
   * Locates the elements at the user-specified points
   */
  virtual void execute();

  /**
   * Performs the actual calculation of the variables at the specified points
   */
  virtual void finalize();

  /**
   * Performs thread joining operations
   * @see SamplerBase
   */
  virtual void threadJoin(const SamplerBase & y);

protected:

  /// The Mesh we're using
  MeshBase & _mesh;

  /// The points to evaluate at (using a parallel vector here to save on point locator calls)
  std::vector<Point> _points;

  /// The ID to use for each point (yes, this is Real on purpose)
  std::vector<Real> _ids;

  /// So we don't have to create and destroy this vector over and over again
  std::vector<Real> _values;

  /// The point locator utilized to find the user-specified locations
  AutoPtr<PointLocatorBase> _point_locator;

  /// Stores the processor ids for the elements located with the point locator
  std::vector<processor_id_type> _root_ids;

  /// Stores the element ids for the elements located with the point locator
  std::vector<dof_id_type> _elem_ids;
};

#endif
