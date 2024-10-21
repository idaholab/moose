//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

/**
 * Utility functions to return the permutation
 * pseudo tensor in 2D, 3D or 4D.
 * eps(i1, i2, ..., iD) =
 *   +1 if (i1, i2, ..., iD) is an even permutation of (0, 1, ..., D)
 *   -1 if (i1, i2, ..., iD) is an odd permutation of (0, 1, ..., D)
 *    0 otherwise
 * The permutation tensor is also known as the Levi-Civita symbol
 */
namespace PermutationTensor
{
/**
 * 2D version
 */
int eps(unsigned int i, unsigned int j);

/**
 * 3D version
 */
int eps(unsigned int i, unsigned int j, unsigned int k);

/**
 * 4D version
 */
int eps(unsigned int i, unsigned int j, unsigned int k, unsigned int l);
}
