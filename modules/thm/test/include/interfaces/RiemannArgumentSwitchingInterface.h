//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef RIEMANNARGUMENTSWITCHINGINTERFACE_H
#define RIEMANNARGUMENTSWITCHINGINTERFACE_H

#include "InputParameters.h"

class RiemannArgumentSwitchingInterface;
class MooseObject;

template <>
InputParameters validParams<RiemannArgumentSwitchingInterface>();

/**
 * Interface class for switching the order of arguments to test the symmetry
 * of Riemann-type interfaces.
 *
 * This class provides a switch that should be used to determine if left and
 * right states should be switched and the normal reversed.
 */
class RiemannArgumentSwitchingInterface
{
public:
  RiemannArgumentSwitchingInterface(const MooseObject * moose_object);

protected:
  /// Switch the left and right arguments?
  const bool _switch_left_and_right;
};

#endif /* RIEMANNARGUMENTSWITCHINGINTERFACE_H */
