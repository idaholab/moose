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

#ifndef DIRACKERNELWAREHOUSE_H
#define DIRACKERNELWAREHOUSE_H

#include "DiracKernel.h"

#include <vector>

/**
 * Typedef to hide implementation details
 */
typedef std::vector<DiracKernel *>::iterator DiracKernelIterator;

/**
 * Holds DiracKernels and provides some services
 */
class DiracKernelWarehouse
{
public:
  DiracKernelWarehouse();
  virtual ~DiracKernelWarehouse();

  DiracKernelIterator diracKernelsBegin();
  DiracKernelIterator diracKernelsEnd();

  void addDiracKernel(DiracKernel *DiracKernel);
    
protected:
  std::vector<DiracKernel *> _dirac_kernels;
};

#endif // DIRACKERNELWAREHOUSE_H
