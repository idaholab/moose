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

#include "MaxVarNDofsPerElem.h"
#include "NonlinearSystem.h"
#include "Problem.h"
#include "Damper.h"

// libmesh includes
#include "libmesh/threads.h"

MaxVarNDofsPerElem::MaxVarNDofsPerElem(FEProblem & feproblem, NonlinearSystem & sys):
    ThreadedElementLoop<ConstElemRange>(feproblem, sys),
    _max(0),
    _dof_map(_system.dofMap())
{
}

// Splitting Constructor
MaxVarNDofsPerElem::MaxVarNDofsPerElem(MaxVarNDofsPerElem & x, Threads::split split):
    ThreadedElementLoop<ConstElemRange>(x, split),
    _max(0),
    _dof_map(x._dof_map)
{
}

MaxVarNDofsPerElem::~MaxVarNDofsPerElem()
{
}

void
MaxVarNDofsPerElem::onElement(const Elem *elem)
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
