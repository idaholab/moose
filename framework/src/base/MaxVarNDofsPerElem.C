//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaxVarNDofsPerElem.h"
#include "NonlinearSystem.h"
#include "Problem.h"
#include "Damper.h"

// libmesh includes
#include "libmesh/threads.h"

MaxVarNDofsPerElem::MaxVarNDofsPerElem(FEProblemBase & feproblem, NonlinearSystemBase & sys)
  : ThreadedElementLoop<ConstElemRange>(feproblem),
    _system(sys),
    _max(0),
    _dof_map(_system.dofMap())
{
}

// Splitting Constructor
MaxVarNDofsPerElem::MaxVarNDofsPerElem(MaxVarNDofsPerElem & x, Threads::split split)
  : ThreadedElementLoop<ConstElemRange>(x, split), _system(x._system), _max(0), _dof_map(x._dof_map)
{
}

MaxVarNDofsPerElem::~MaxVarNDofsPerElem() {}

void
MaxVarNDofsPerElem::onElement(const Elem * elem)
{
  for (unsigned int var = 0; var < _system.nVariables(); var++)
  {
    _dof_map.dof_indices(elem, _dof_indices, var);

    _max = std::max(_max, _dof_indices.size());
  }
}

void
MaxVarNDofsPerElem::join(const MaxVarNDofsPerElem & y)
{
  _max = std::max(_max, y._max);
}
