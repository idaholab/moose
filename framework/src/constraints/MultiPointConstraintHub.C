//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiPointConstraintHub.h"

#include "Constraint.h"
#include "NonlinearSystemBase.h"

MultiPointConstraintHub::MultiPointConstraintHub(NonlinearSystemBase & sys,
                                                 SystemBase & constrained_system)
  : _sys(sys), _constrained_sys(constrained_system)
{
}

void
MultiPointConstraintHub::constrain()
{
  libMesh::DofMap & dof_map = _constrained_sys.dofMap();
  for (const auto & constraint : _sys.getConstraintWarehouse().getActiveObjects())
    if (constraint->usesExactConstraints())
      constraint->addExactConstraints(dof_map);
}

void
MultiPointConstraintHub::reinit()
{
  _constrained_sys.system().reinit_constraints();
}
