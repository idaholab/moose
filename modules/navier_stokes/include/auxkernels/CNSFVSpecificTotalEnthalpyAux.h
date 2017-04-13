/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef CNSFVSPECIFICTOTALENTHALPYAUX_H
#define CNSFVSPECIFICTOTALENTHALPYAUX_H

#include "AuxKernel.h"

class CNSFVSpecificTotalEnthalpyAux;

template <>
InputParameters validParams<CNSFVSpecificTotalEnthalpyAux>();

/**
 * An aux kernel for calculating specific total enthalpy
 */
class CNSFVSpecificTotalEnthalpyAux : public AuxKernel
{
public:
  CNSFVSpecificTotalEnthalpyAux(const InputParameters & parameters);
  virtual ~CNSFVSpecificTotalEnthalpyAux() {}

protected:
  virtual Real computeValue();

  const MaterialProperty<Real> & _enth;
};

#endif
