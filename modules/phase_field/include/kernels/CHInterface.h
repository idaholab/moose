/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CHINTERFACE_H
#define CHINTERFACE_H

#include "CHInterfaceBase.h"

/**
 * This is the Cahn-Hilliard equation base class that implements the interfacial
 * or gradient energy term of the equation. With a scalar (isotropic) mobility.
 */
class CHInterface : public CHInterfaceBase<Real>
{
public:
  CHInterface(const InputParameters & parameters);
};

template <>
InputParameters validParams<CHInterface>();

#endif // CHINTERFACE_H
