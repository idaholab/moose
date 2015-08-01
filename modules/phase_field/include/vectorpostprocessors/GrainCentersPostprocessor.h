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

#ifndef GRAINCENTERSPOSTPROCESSOR_H
#define GRAINCENTERSPOSTPROCESSOR_H

#include "GeneralVectorPostprocessor.h"
#include "ComputeGrainCenterUserObject.h"

//Forward Declarations
class GrainCentersPostprocessor;

template<>
InputParameters validParams<GrainCentersPostprocessor>();

/**
 *  GrainCentersPostprocessor is a type of VectorPostprocessor that outputs center and volume of grains
 *  calculated in GrainCenterUserObject.
 */

class GrainCentersPostprocessor :
  public GeneralVectorPostprocessor
{
public:
  /**
    * Class constructor
    * @param name The name of the object
    * @param parameters The input parameters
    */
  GrainCentersPostprocessor(const InputParameters & parameters);

  /**
   * Destructor
   */
  virtual ~GrainCentersPostprocessor() {}

  /**
   * Initialize, clears the postprocessor vector
   */
  virtual void initialize(){};

  /**
   * Populates the postprocessor vector of values with the supplied postprocessors
   */
  virtual void execute();

  ///@{
  /**
   * no-op because the postprocessors are already parallel consistent
   */
  virtual void finalize() {}
  virtual void threadJoin(const UserObject &) {}
  ///@}

protected:
  /// The VectorPostprocessorValue object where the results are stored
  VectorPostprocessorValue & _grain_volume_center_vector;

  /// Userobject that calculates volumes and centers of grains
  const ComputeGrainCenterUserObject & _grain_data;
  /// Extracting grain volumes from Userobject
  const std::vector<Real> & _grain_volumes;
  /// Extracting grain centers from Userobject
  const std::vector<Point> & _grain_centers;

  unsigned int _total_grains;
};

#endif //GRAINCENTERSPOSTPROCESSOR_H
