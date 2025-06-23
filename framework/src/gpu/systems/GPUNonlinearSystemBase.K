//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUKernelBase.h"
#include "GPUNodalKernelBase.h"
#include "GPUNodalBCBase.h"
#include "GPUIntegratedBCBase.h"

#include "MaterialBase.h"
#include "NonlinearSystemBase.h"
#include "MooseVariableFieldBase.h"
#include "FEProblemBase.h"

void
NonlinearSystemBase::addGPUKernel(const std::string & kernel_name,
                                  const std::string & name,
                                  InputParameters & parameters)
{
  // Create the kernel object via the factory and add to warehouse
  std::shared_ptr<GPUKernelBase> kernel =
      _factory.create<GPUKernelBase>(kernel_name, name, parameters);
  _gpu_kernels.addObject(kernel, 0);
  postAddResidualObject(*kernel);
}

void
NonlinearSystemBase::addGPUNodalKernel(const std::string & kernel_name,
                                       const std::string & name,
                                       InputParameters & parameters)
{
  // Create the kernel object via the factory and add to warehouse
  std::shared_ptr<GPUNodalKernelBase> kernel =
      _factory.create<GPUNodalKernelBase>(kernel_name, name, parameters);
  _gpu_nodal_kernels.addObject(kernel, 0);
  postAddResidualObject(*kernel);
}

void
NonlinearSystemBase::addGPUBoundaryCondition(const std::string & bc_name,
                                             const std::string & name,
                                             InputParameters & parameters)
{
  // Create the object
  std::shared_ptr<GPUBoundaryCondition> bc =
      _factory.create<GPUBoundaryCondition>(bc_name, name, parameters, 0);
  postAddResidualObject(*bc);

  // Active BoundaryIDs for the object
  const std::set<BoundaryID> & boundary_ids = bc->boundaryIDs();
  auto bc_var = dynamic_cast<const MooseVariableFieldBase *>(&bc->variable());
  _vars[0].addBoundaryVar(boundary_ids, bc_var);

  // Cast to the various types of BCs
  std::shared_ptr<GPUNodalBCBase> nbc = std::dynamic_pointer_cast<GPUNodalBCBase>(bc);
  std::shared_ptr<GPUIntegratedBCBase> ibc = std::dynamic_pointer_cast<GPUIntegratedBCBase>(bc);

  // GPUNodalBCBase
  if (nbc)
  {
    if (!nbc->variable().isNodal())
      mooseError("Trying to use nodal boundary condition '",
                 nbc->name(),
                 "' on a non-nodal variable '",
                 nbc->variable().name(),
                 "'.");

    _gpu_nodal_bcs.addObject(nbc);
    _vars[0].addBoundaryVars(boundary_ids, nbc->getCoupledVars());

    // GPUDirichletBCs that are preset
    if (nbc->preset())
      _gpu_preset_nodal_bcs.addObject(nbc);
  }

  // GPUIntegratedBCBase
  else if (ibc)
  {
    _gpu_integrated_bcs.addObject(ibc);
    _vars[0].addBoundaryVars(boundary_ids, ibc->getCoupledVars());
  }

  else
    mooseError("Unknown GPUBoundaryCondition type for object named ", bc->name());
}
