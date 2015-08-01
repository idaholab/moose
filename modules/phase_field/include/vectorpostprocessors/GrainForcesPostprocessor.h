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

#ifndef GRAINFORCESPOSTPROCESSOR_H
#define GRAINFORCESPOSTPROCESSOR_H

#include "GeneralVectorPostprocessor.h"
#include "GrainForceAndTorqueInterface.h"

//Forward Declarations
class GrainForcesPostprocessor;

template<>
InputParameters validParams<GrainForcesPostprocessor>();

/**
 *  GrainForcesPostprocessor is a type of VectorPostprocessor that outputs the
 *  force and torque values calculated in UserObjects.
 */

class GrainForcesPostprocessor :
  public GeneralVectorPostprocessor
{
public:
  /**
    * Class constructor
    * @param name The name of the object
    * @param parameters The input parameters
    */
  GrainForcesPostprocessor(const InputParameters & parameters);

  /**
   * Destructor
   */
  virtual ~GrainForcesPostprocessor() {}

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
  VectorPostprocessorValue & _grain_force_torque_vector;

  /// UserobjectInterface for getting force and torque values from UserObjects
  const GrainForceAndTorqueInterface & _grain_force_torque;
  /// Extracting forces from Userobject
  const std::vector<RealGradient> & _grain_forces;
  /// Extracting torques from Userobject
  const std::vector<RealGradient> & _grain_torques;
  /// Extracting derivative of forces from Userobject
  const std::vector<RealGradient> & _grain_force_derivatives;
  /// Extracting derivative of torques from Userobject
  const std::vector<RealGradient> & _grain_torque_derivatives;

  unsigned int _total_grains;
};

#endif //GRAINFORCESPOSTPROCESSOR_H
