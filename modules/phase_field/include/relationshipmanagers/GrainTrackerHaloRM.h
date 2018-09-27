//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef GRAINTRACKERHALORM_H
#define GRAINTRACKERHALORM_H

#include "ElementPointNeighbors.h"

// Forward declarations
class GrainTrackerHaloRM;

template <>
InputParameters validParams<GrainTrackerHaloRM>();

/**
 * GrainTrackerHaloRM is used to ensure a level of elements at least as thick as the desired halo
 * level is available on each processor's partition to support detecting grain interaction.
 */
class GrainTrackerHaloRM : public ElementPointNeighbors
{
public:
  GrainTrackerHaloRM(const InputParameters & parameters);

  virtual std::string getInfo() const override;
};

#endif /* GRAINTRACKERHALORM_H */
