//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef KERNELWAREHOUSE_H
#define KERNELWAREHOUSE_H

// MOOSE includes
#include "MooseObjectWarehouse.h"
#include "MooseTypes.h"
#include "MooseError.h"

// Forward declarations
class KernelBase;
class ScalarKernel;
class TimeKernel;

/**
 * Holds kernels and provides some services
 */
class KernelWarehouse : public MooseObjectWarehouse<KernelBase>
{
public:
  KernelWarehouse();

  /**
   * Add Kernel to the storage structure
   */
  void addObject(std::shared_ptr<KernelBase> object, THREAD_ID tid = 0) override;

  ///@{
  /**
   * Methods for checking/getting variable kernels for a variable and SubdomainID
   */
  bool hasActiveVariableBlockObjects(unsigned int variable_id,
                                     SubdomainID block_id,
                                     THREAD_ID tid = 0) const;
  const std::vector<std::shared_ptr<KernelBase>> & getActiveVariableBlockObjects(
      unsigned int variable_id, SubdomainID block_id, THREAD_ID tid = 0) const;
  ///@}

  /**
   * Update the active status of Kernels
   */
  virtual void updateActive(THREAD_ID tid = 0) override;

protected:
  /// Variable based storage
  std::map<unsigned int, MooseObjectWarehouse<KernelBase>> _variable_kernel_storage;
};

#endif // KERNELWAREHOUSE_H
