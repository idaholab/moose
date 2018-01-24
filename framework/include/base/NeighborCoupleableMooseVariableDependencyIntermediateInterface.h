//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NEIGHBORCOUPLEABLEMOOSEVARIABLEDEPENDENCYINTERMEDIATEINTERFACE_H
#define NEIGHBORCOUPLEABLEMOOSEVARIABLEDEPENDENCYINTERMEDIATEINTERFACE_H

#include "NeighborCoupleable.h"
#include "ScalarCoupleable.h"
#include "NeighborMooseVariableInterface.h"
#include "MooseVariableDependencyInterface.h"

/**
 * Intermediate base class that ties together all the interfaces for getting
 * MooseVariables with the MooseVariableDependencyInterface
 */
class NeighborCoupleableMooseVariableDependencyIntermediateInterface
    : public NeighborCoupleable,
      public ScalarCoupleable,
      public NeighborMooseVariableInterface,
      public MooseVariableDependencyInterface
{
public:
  NeighborCoupleableMooseVariableDependencyIntermediateInterface(const MooseObject * moose_object,
                                                                 bool nodal,
                                                                 bool neighbor_nodal)
    : NeighborCoupleable(moose_object, nodal, neighbor_nodal),
      ScalarCoupleable(moose_object),
      NeighborMooseVariableInterface(moose_object, nodal)
  {
    const std::vector<MooseVariable *> & coupled_vars = getCoupledMooseVars();
    for (unsigned int i = 0; i < coupled_vars.size(); i++)
      addMooseVariableDependency(coupled_vars[i]);

    addMooseVariableDependency(mooseVariable());
  }
};

#endif // NEIGHBORCOUPLEABLEMOOSEVARIABLEDEPENDENCYINTERMEDIATEINTERFACE_H
