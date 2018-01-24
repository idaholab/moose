//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KernelWarehouse.h"

// MOOSE includes
#include "KernelBase.h"
#include "MooseVariable.h"
#include "TimeDerivative.h"
#include "TimeKernel.h"

KernelWarehouse::KernelWarehouse() : MooseObjectWarehouse<KernelBase>() {}

void
KernelWarehouse::addObject(std::shared_ptr<KernelBase> object, THREAD_ID tid)
{
  // Add object to the general storage
  MooseObjectWarehouse<KernelBase>::addObject(object, tid);

  // Add object to the variable based storage
  _variable_kernel_storage[object->variable().number()].addObject(object, tid);
}

bool
KernelWarehouse::hasActiveVariableBlockObjects(unsigned int variable_id,
                                               SubdomainID block_id,
                                               THREAD_ID tid) const
{
  checkThreadID(tid);
  std::map<unsigned int, MooseObjectWarehouse<KernelBase>>::const_iterator iter =
      _variable_kernel_storage.find(variable_id);
  return (iter != _variable_kernel_storage.end() &&
          iter->second.hasActiveBlockObjects(block_id, tid));
}

const std::vector<std::shared_ptr<KernelBase>> &
KernelWarehouse::getActiveVariableBlockObjects(unsigned int variable_id,
                                               SubdomainID block_id,
                                               THREAD_ID tid) const
{
  checkThreadID(tid);
  const auto iter = _variable_kernel_storage.find(variable_id);
  mooseAssert(iter != _variable_kernel_storage.end(),
              "Unable to located variable kernels for the given variable id: " << variable_id
                                                                               << ".");
  return iter->second.getActiveBlockObjects(block_id, tid);
}

void
KernelWarehouse::updateActive(THREAD_ID tid)
{
  MooseObjectWarehouse<KernelBase>::updateActive(tid);

  for (auto & it : _variable_kernel_storage)
    it.second.updateActive(tid);
}
