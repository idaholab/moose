/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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
