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
 * Holds DiracKernels and provides some services
 */
class DiracKernelWarehouse
{
public:
  DiracKernelWarehouse();
  virtual ~DiracKernelWarehouse();

  // Setup /////
  void initialSetup();
  void timestepSetup();
  void residualSetup();
  void jacobianSetup();

  /**
   * Get the list of all dirac kernels
   * @return The list of all dirac kernels
   */
  const std::vector<DiracKernel *> & all() { return _dirac_kernels; }

  /**
   * Adds a dirac kernel
   * @param DiracKernel Kernel being added
   */
  void addDiracKernel(DiracKernel *DiracKernel);

protected:
  std::vector<DiracKernel *> _dirac_kernels;            ///< All dirac kernels
};

#endif // DIRACKERNELWAREHOUSE_H
