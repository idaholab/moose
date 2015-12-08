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

// MOOSE includes
#include "KernelStorage.h"
#include "TimeDerivative.h"
#include "KernelBase.h"
#include "TimeKernel.h"

KernelStorage::KernelStorage() :
    MooseObjectStorage<KernelBase>()
{
}


void
KernelStorage::addObject(MooseSharedPointer<KernelBase> object, THREAD_ID tid)
{
  // Add object to the general storage
  MooseObjectStorage<KernelBase>::addObject(object, tid);

  // Add object to the variable based storage
  _variable_kernel_storage[object->variable().number()].addObject(object, tid);
}


bool
KernelStorage::hasActiveVariableBlockObjects(unsigned int variable_id, SubdomainID block_id, THREAD_ID tid) const
{
  checkThreadID(tid);
  std::map<unsigned int, MooseObjectStorage<KernelBase> >::const_iterator iter = _variable_kernel_storage.find(variable_id);
  return (iter != _variable_kernel_storage.end() && iter->second.hasActiveBlockObjects(block_id, tid));
}


const std::vector<MooseSharedPointer<KernelBase> > &
KernelStorage::getActiveVariableBlockObjects(unsigned int variable_id, SubdomainID block_id, THREAD_ID tid) const
{
  checkThreadID(tid);
  std::map<unsigned int, MooseObjectStorage<KernelBase> >::const_iterator iter = _variable_kernel_storage.find(variable_id);
  mooseAssert(iter != _variable_kernel_storage.end(), "Unable to located variable kernels for the given variable id: " << variable_id << ".");
  return iter->second.getActiveBlockObjects(block_id, tid);
}


void
KernelStorage::updateActive(THREAD_ID tid)
{
  MooseObjectStorage<KernelBase>::updateActive(tid);

  std::map<unsigned int, MooseObjectStorage<KernelBase> >::iterator it;
  for (it = _variable_kernel_storage.begin(); it != _variable_kernel_storage.end(); ++it)
    it->second.updateActive(tid);
}
