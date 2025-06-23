//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUTypes.h"

#include "FEProblemBase.h"
#include "NonlinearSystemBase.h"
#include "AuxiliarySystem.h"
#include "MaterialBase.h"

void
FEProblemBase::initGPU()
{
  // Initialize GPU assembly

  _gpu_assembly.init();
  _gpu_assembly.initQuadrature();
  _gpu_assembly.initShape();

  // Initialize GPU systems

  unsigned int max_system_number = 0;

  for (unsigned int s = 0; s < numNonlinearSystems(); ++s)
    max_system_number = std::max(max_system_number, getNonlinearSystemBase(s).number());

  max_system_number = std::max(max_system_number, getAuxiliarySystem().number());

  // GPUSystem does not have a default constructor, so this simply allocates buffer to which we
  // should do placement new later
  _gpu_systems.create(max_system_number + 1);

  for (unsigned int s = 0; s < numNonlinearSystems(); ++s)
  {
    auto & nl = getNonlinearSystemBase(s);
    new (&_gpu_systems[nl.number()]) GPUSystem(nl);
    _gpu_systems[nl.number()].init();
  }

  auto & aux = getAuxiliarySystem();
  new (&_gpu_systems[aux.number()]) GPUSystem(aux);
  _gpu_systems[aux.number()].init();
}
