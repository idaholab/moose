/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef RANDOMEULERANGLEPROVIDER_H
#define RANDOMEULERANGLEPROVIDER_H

#include "EulerAngleProvider.h"
#include "MooseRandom.h"

// Forward declaration
class RandomEulerAngleProvider;
class GrainTrackerInterface;

template <>
InputParameters validParams<RandomEulerAngleProvider>();

/**
 * Assign random Euler angles to each grains
 */
class RandomEulerAngleProvider : public EulerAngleProvider
{
public:
  RandomEulerAngleProvider(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override {}
  virtual void finalize() override {}

  virtual const EulerAngles & getEulerAngles(unsigned int) const override;
  virtual unsigned int getGrainNum() const override;

protected:
  const GrainTrackerInterface & _grain_tracker;
  std::vector<EulerAngles> _angles;

  MooseRandom _random;
};

#endif // RANDOMEULERANGLEPROVIDER_H
