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

#include "PermutationTensor.h"

PermutationTensor::PermutationTensor()
{}

int
PermutationTensor::eps(unsigned int i, unsigned int j)
{
  if (i==0 && j==1)
    return 1;
  else if (i==1 && j==0)
    return -1;
  return 0;
}

int
PermutationTensor::eps(unsigned int i, unsigned int j, unsigned int k)
{
  if (i==0 && j>0 && k>0)
    return eps(j-1, k-1);
  else if (j==0 && i>0 && k>0)
    return -eps(i-1, k-1);
  else if (k==0 && i>0 && j>0)
    return eps(i-1, j-1);
  return 0;
}

int
PermutationTensor::eps(unsigned int i, unsigned int j, unsigned int k, unsigned int l)
{
  if (i==0 && j>0 && k>0 && l>0)
    return eps(j-1, k-1, l-1);
  else if (j==0 && i>0 && k>0 && l>0)
    return -eps(i-1, k-1, l-1);
  else if (k==0 && i>0 && j>0 && l>0)
    return eps(i-1, j-1, l-1);
  else if (l==0 && i>0 && j>0 && k>0)
    return -eps(i-1, j-1, k-1);
  return 0;
}

