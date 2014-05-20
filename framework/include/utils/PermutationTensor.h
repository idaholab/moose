/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef PERMUTATIONTENSOR_H
#define PERMUTATIONTENSOR_H

/**
 * Utility functions to return the permutation
 * pseudo tensor in 2D, 3D or 4D.
 * eps(i1, i2, ..., iD) = 
 *   +1 if (i1, i2, ..., iD) is an even permutation of (0, 1, ..., D)
 *   -1 if (i1, i2, ..., iD) is an odd permutation of (0, 1, ..., D)
 *    0 otherwise
 * The permutation tensor is also known as the Levi-Civita symbol
 */
class PermutationTensor
{
 public:
  PermutationTensor();

  /**
   * 2D version
   */
  static int eps(unsigned int i, unsigned int j);

  /**
   * 3D version
   */
  static int eps(unsigned int i, unsigned int j, unsigned int k);

  /**
   * 4D version
   */
  static int eps(unsigned int i, unsigned int j, unsigned int k, unsigned int l);
};

#endif // PERMUTATIONTENSOR_H
