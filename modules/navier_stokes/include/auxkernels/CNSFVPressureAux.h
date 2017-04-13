/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef CNSFVPRESSUREAUX_H
#define CNSFVPRESSUREAUX_H

#include "AuxKernel.h"

class CNSFVPressureAux;

template <>
InputParameters validParams<CNSFVPressureAux>();

/**
 * An aux kernel for calculating pressure
 */
class CNSFVPressureAux : public AuxKernel
{
public:
  CNSFVPressureAux(const InputParameters & parameters);
  virtual ~CNSFVPressureAux() {}

protected:
  virtual Real computeValue();

  const MaterialProperty<Real> & _pres;
};

#endif
