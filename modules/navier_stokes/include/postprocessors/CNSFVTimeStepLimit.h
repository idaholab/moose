/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef CNSFVTIMESTEPLIMIT_H
#define CNSFVTIMESTEPLIMIT_H

#include "ElementPostprocessor.h"

class CNSFVTimeStepLimit;

template <>
InputParameters validParams<CNSFVTimeStepLimit>();

/**
 * A PostProcessor object to calculate the allowable time step size for the CNS equations
 */
class CNSFVTimeStepLimit : public ElementPostprocessor
{
public:
  CNSFVTimeStepLimit(const InputParameters & parameters);
  virtual ~CNSFVTimeStepLimit();

  virtual void initialize();

  virtual void execute();

  virtual Real getValue();

  virtual void threadJoin(const UserObject & uo);

protected:
  unsigned int _dim;

  /// the value of dt
  Real _value;

  /// user-input coefficient
  Real _cfl;

  /// velocity magnitude
  const MaterialProperty<Real> & _vmag;

  /// speed of sound
  const MaterialProperty<Real> & _csou;
};

#endif
