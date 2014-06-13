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

#include <vector>

class DiracKernel;

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
  const std::vector<DiracKernel *> & all() const { return _dirac_kernels; }

  /**
   * Adds a Dirac kernel
   * @param kernel The DiracKernel being added
   */
  void addDiracKernel(DiracKernel * kernel);

protected:
  /// All dirac kernels
  std::vector<DiracKernel *> _dirac_kernels;
};

#endif // DIRACKERNELWAREHOUSE_H
