/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef MIXEDMODEEFFECTIVEK_H
#define MIXEDMODEEFFECTIVEK_H

#include "GeneralPostprocessor.h"

//Forward Declarations
class MixedModeEffectiveK;

template<>
InputParameters validParams<MixedModeEffectiveK>();

class MixedModeEffectiveK : public GeneralPostprocessor
{
public:
  MixedModeEffectiveK(const std::string & name, InputParameters parameters);

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

#endif //MIXEDMODEEFFECTIVEK_H
