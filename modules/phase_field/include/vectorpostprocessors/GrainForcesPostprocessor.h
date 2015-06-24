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
#include "ComputeGrainForceAndTorque.h"

//Forward Declarations
class GrainForcesPostprocessor;

template<>
InputParameters validParams<GrainForcesPostprocessor>();

/**
 *  GrainForcesPostprocessor is a type of VectorPostprocessor that outputs the
 *  values of an arbitrary user-specified set of postprocessors as a vector in the order specified by the user.
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
  GrainForcesPostprocessor(const std::string & name, InputParameters parameters);

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

  /// The vector of PostprocessorValue objects that are used to get the values of the postprocessors
  const ComputeGrainForceAndTorque & _grain_force_torque;
  const std::vector<RealGradient> & _grain_forces;
  const std::vector<RealGradient> & _grain_torques;

  unsigned int _total_grains;
};

#endif //GRAINFORCESPOSTPROCESSOR_H
