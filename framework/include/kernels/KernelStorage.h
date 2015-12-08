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

#ifndef KERNELSTORAGE_H
#define KERNELSTORAGE_H

// MOOSE includes
#include "MooseObjectStorage.h"
#include "MooseTypes.h"
#include "MooseError.h"

// Forward declarations
class KernelBase;
class ScalarKernel;
class TimeKernel;

/**
 * Holds kernels and provides some services
 */
class KernelStorage : public MooseObjectStorage<KernelBase>
{
public:
  KernelStorage();

  /**
   * Add Kernel to the storage structure
   */
  virtual void addObject(MooseSharedPointer<KernelBase> object, THREAD_ID tid = 0);

  ///@{
  /**
   * Methods for checking/getting variable kernels for a variable and SubdomainID
   */
  bool hasActiveVariableBlockObjects(unsigned int variable_id, SubdomainID block_id, THREAD_ID tid = 0) const;
  const std::vector<MooseSharedPointer<KernelBase> > & getActiveVariableBlockObjects(unsigned int variable_id, SubdomainID block_id, THREAD_ID tid = 0) const;
  ///@}

  /**
   * Update the active status of Kernels
   */
  virtual void updateActive(THREAD_ID tid = 0);

protected:

  /// Variable based storage
  std::map<unsigned int, MooseObjectStorage<KernelBase> > _variable_kernel_storage;

};

#endif // KERNELSTORAGE_H
