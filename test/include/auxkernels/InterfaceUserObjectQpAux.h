//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef InterfaceUserObjectQpAux_H
#define InterfaceUserObjectQpAux_H

#include "AuxKernel.h"
#include "InterfaceUO_QP.h"

// Forward Declarations
class InterfaceUserObjectQpAux;

template <>
InputParameters validParams<InterfaceUserObjectQpAux>();

/**
 * Self auxiliary value
 */
class InterfaceUserObjectQpAux : public AuxKernel
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  InterfaceUserObjectQpAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;
  const InterfaceUO_QP & _interface_uo;
};

#endif // InterfaceUserObjectQpAux_H
