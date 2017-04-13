/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CONTACTPRESSUREAUX_H
#define CONTACTPRESSUREAUX_H

#include "AuxKernel.h"

class NodalArea;
class PenetrationLocator;

class ContactPressureAux : public AuxKernel
{
public:
  ContactPressureAux(const InputParameters & parameters);

  virtual ~ContactPressureAux();

protected:
  virtual Real computeValue();

  const VariableValue & _nodal_area;
  const PenetrationLocator & _penetration_locator;
};

template <>
InputParameters validParams<ContactPressureAux>();

#endif
