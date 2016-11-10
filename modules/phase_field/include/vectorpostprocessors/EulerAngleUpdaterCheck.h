/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef EULERANGLEUPDATERCHECK_H
#define EULERANGLEUPDATERCHECK_H

#include "GeneralVectorPostprocessor.h"

//Forward declaration
class EulerAngleUpdaterCheck;
class EulerAngleUpdater;
class EulerAngleProvider;
class GrainTrackerInterface;
class GrainForceAndTorqueInterface;
class RotationTensor;

template<>
InputParameters validParams<EulerAngleUpdaterCheck>();

/**
 * Update Euler angles of each grains after rigid body rotation
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

#endif //EULERANGLEUPDATERCHECK_H
