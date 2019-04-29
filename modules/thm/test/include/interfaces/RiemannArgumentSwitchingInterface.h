#pragma once

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
