/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef EULERANGLEUPDATER_H
#define EULERANGLEUPDATER_H

#include "EulerAngleProvider.h"

//Forward declaration
class EulerAngleUpdater;
class RotationTensor;
class GrainTrackerInterface;
class GrainForceAndTorqueInterface;

template<>
InputParameters validParams<EulerAngleUpdater>();

/**
 * Update Euler angles of each grains after rigid body rotation
 */
class EulerAngleUpdater : public EulerAngleProvider
{
public:
  EulerAngleUpdater(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override {}
  virtual void finalize() override {}

  virtual const EulerAngles & getEulerAngles(unsigned int) const override;
  virtual const EulerAngles & getEulerAnglesOld(unsigned int) const;
  virtual unsigned int getGrainNum() const override;

protected:
  const GrainTrackerInterface & _grain_tracker;
  const EulerAngleProvider & _euler;
  const GrainForceAndTorqueInterface & _grain_torque;
  const VectorPostprocessorValue & _grain_volumes;

  const Real _mr;
  bool _first_time;

  std::vector<EulerAngles> _angles;
  std::vector<EulerAngles> _angles_old;
};

#endif //EULERANGLEUPDATER_H
