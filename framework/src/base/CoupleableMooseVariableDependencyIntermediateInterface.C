//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupleableMooseVariableDependencyIntermediateInterface.h"

CoupleableMooseVariableDependencyIntermediateInterface::
    CoupleableMooseVariableDependencyIntermediateInterface(const MooseObject * moose_object,
                                                           bool nodal)
  : Coupleable(moose_object, nodal),
    ScalarCoupleable(moose_object),
    MooseVariableInterface(moose_object, nodal)
{
  for (MooseVariable * coupled_var : getCoupledMooseVars())
    addMooseVariableDependency(coupled_var);

  addMooseVariableDependency(mooseVariable());
}
