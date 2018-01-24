//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
