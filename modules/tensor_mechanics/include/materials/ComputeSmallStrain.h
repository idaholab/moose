//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTESMALLSTRAIN_H
#define COMPUTESMALLSTRAIN_H

#include "ComputeStrainBase.h"

class ComputeSmallStrain;

template <>
InputParameters validParams<ComputeSmallStrain>();

/**
 * ComputeSmallStrain defines a strain tensor, assuming small strains.
 */
class ComputeSmallStrain : public ComputeStrainBase
{
public:
  ComputeSmallStrain(const InputParameters & parameters);

protected:
  virtual void computeProperties() override;
};

#endif // COMPUTESMALLSTRAIN_H
