//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INTERFACEUSEROBJECTQPAUX_H
#define INTERFACEUSEROBJECTQPAUX_H

#include "AuxKernel.h"
#include "InterfaceValueUO_QP.h"

// Forward Declarations
class InterfaceValueUO_QP_Aux;

template <>
InputParameters validParams<InterfaceValueUO_QP_Aux>();

/**
 * AuxKernel creating an AuxVariable from values stored in an InterfaceValueUO_QP
 */
class InterfaceValueUO_QP_Aux : public AuxKernel
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  InterfaceValueUO_QP_Aux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;
  const InterfaceValueUO_QP & _interface_uo;
};

#endif // INTERFACEUSEROBJECTQPAUX_H
