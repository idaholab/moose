//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

namespace Moose::Kokkos
{

/**
 * A block of consecutive derivative slots of a pointwise AD object, holding the derivatives with
 * respect to one quadrature point quantity of one nonlinear variable
 */
struct ADSlotBlock
{
  /// System number of the variable
  unsigned int sys;
  /// Variable number of the variable in its system
  unsigned int var;
  /// First slot of the block
  unsigned int first;
  /// Whether the block holds the gradient, of AD_GRADIENT_SLOTS slots, or the value, of one slot
  bool gradient;
};

} // namespace Moose::Kokkos
