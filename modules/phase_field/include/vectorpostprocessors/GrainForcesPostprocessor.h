/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef GRAINFORCESPOSTPROCESSOR_H
#define GRAINFORCESPOSTPROCESSOR_H

#include "GeneralVectorPostprocessor.h"

//Forward Declarations
class GrainForcesPostprocessor;
class GrainForceAndTorqueInterface;

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
  GrainForcesPostprocessor(const InputParameters & parameters);

  virtual ~GrainForcesPostprocessor() {}
  virtual void initialize() {};
  virtual void execute();

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
