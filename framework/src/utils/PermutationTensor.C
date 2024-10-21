//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PermutationTensor.h"

int
PermutationTensor::eps(unsigned int i, unsigned int j)
{
  if (i == 0 && j == 1)
    return 1;
  else if (i == 1 && j == 0)
    return -1;
  return 0;
}

int
PermutationTensor::eps(unsigned int i, unsigned int j, unsigned int k)
{
  if (i == 0 && j > 0 && k > 0)
    return eps(j - 1, k - 1);
  else if (j == 0 && i > 0 && k > 0)
    return -eps(i - 1, k - 1);
  else if (k == 0 && i > 0 && j > 0)
    return eps(i - 1, j - 1);
  return 0;
}

int
PermutationTensor::eps(unsigned int i, unsigned int j, unsigned int k, unsigned int l)
{
  if (i == 0 && j > 0 && k > 0 && l > 0)
    return eps(j - 1, k - 1, l - 1);
  else if (j == 0 && i > 0 && k > 0 && l > 0)
    return -eps(i - 1, k - 1, l - 1);
  else if (k == 0 && i > 0 && j > 0 && l > 0)
    return eps(i - 1, j - 1, l - 1);
  else if (l == 0 && i > 0 && j > 0 && k > 0)
    return -eps(i - 1, j - 1, k - 1);
  return 0;
}
