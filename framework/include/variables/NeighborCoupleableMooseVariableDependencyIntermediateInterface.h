//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NeighborCoupleable.h"
#include "ScalarCoupleable.h"
#include "NeighborMooseVariableInterface.h"
#include "MooseVariableDependencyInterface.h"
#include "InputParameters.h"

/**
 * Intermediate base class that ties together all the interfaces for getting
 * MooseVariables with the MooseVariableDependencyInterface
 */
class NeighborCoupleableMooseVariableDependencyIntermediateInterface
  : public NeighborCoupleable,
    public ScalarCoupleable,
    public MooseVariableDependencyInterface
{
public:
  static InputParameters validParams() { return emptyInputParameters(); }

  NeighborCoupleableMooseVariableDependencyIntermediateInterface(const MooseObject * moose_object,
                                                                 bool nodal,
                                                                 bool neighbor_nodal,
                                                                 bool is_fv = false)
    : NeighborCoupleable(moose_object, nodal, neighbor_nodal, is_fv),
      ScalarCoupleable(moose_object),
      MooseVariableDependencyInterface(moose_object)
  {
    for (MooseVariableFEBase * coupled_var : getCoupledMooseVars())
      addMooseVariableDependency(coupled_var);
  }
};
