/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CHINTERFACEANISO_H
#define CHINTERFACEANISO_H

#include "CHInterfaceBase.h"

/**
 * This is the Cahn-Hilliard equation base class that implements the interfacial
 * or gradient energy term of the equation. With a scalar (isotropic) mobility.
 */
class CHInterfaceAniso : public CHInterfaceBase<RealTensorValue>
{
public:
  CHInterfaceAniso(const InputParameters & parameters);
};

template <>
InputParameters validParams<CHInterfaceAniso>();

#endif // CHINTERFACEANISO_H
