/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef EULERANGLEUPDATERCHECK_H
#define EULERANGLEUPDATERCHECK_H

#include "GeneralVectorPostprocessor.h"

// Forward declaration
class EulerAngleUpdaterCheck;
class EulerAngleUpdater;
class EulerAngleProvider;
class GrainTrackerInterface;
class GrainForceAndTorqueInterface;
class RotationTensor;

template <>
InputParameters validParams<EulerAngleUpdaterCheck>();

/**
 * This is a unit test to check the correctness of the updated euler angles
 * An unit vector is rotated as per old euler angles first and then due to the applied torque
 * The final rotated vector is cross checked with the rotated vector as per updated euler angles
 */
class EulerAngleUpdaterCheck : public GeneralVectorPostprocessor
{
public:
  EulerAngleUpdaterCheck(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override {}
  virtual void finalize() override {}

  VectorPostprocessorValue & _diff;

protected:
  const GrainTrackerInterface & _grain_tracker;
  const EulerAngleUpdater & _euler;
  const GrainForceAndTorqueInterface & _grain_torque;
  const VectorPostprocessorValue & _grain_volumes;

  const Real _mr;

  std::vector<RealVectorValue> _angles;
  std::vector<RealVectorValue> _angles_old;
};

#endif // EULERANGLEUPDATERCHECK_H
