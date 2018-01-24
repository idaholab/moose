//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MIXEDMODEEQUIVALENTK_H
#define MIXEDMODEEQUIVALENTK_H

#include "GeneralPostprocessor.h"

// Forward Declarations
class MixedModeEquivalentK;

template <>
InputParameters validParams<MixedModeEquivalentK>();

class MixedModeEquivalentK : public GeneralPostprocessor
{
public:
  MixedModeEquivalentK(const InputParameters & parameters);

  virtual void initialize() {}
  virtual void execute() {}

  /**
   * This will return the degrees of freedom in the system.
   */
  virtual Real getValue();

protected:
  const PostprocessorValue & _ki_value;
  const PostprocessorValue & _kii_value;
  const PostprocessorValue & _kiii_value;
  Real _poissons_ratio;
};

#endif // MIXEDMODEEQUIVALENTK_H
