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
class NeighborCoupleableMooseVariableDependencyIntermediateInterface :
  public NeighborCoupleable,
  public ScalarCoupleable,
  public NeighborMooseVariableInterface,
  public MooseVariableDependencyInterface
{
public:
  NeighborCoupleableMooseVariableDependencyIntermediateInterface(const MooseObject * moose_object, bool nodal, bool neighbor_nodal) :
    NeighborCoupleable(moose_object, nodal, neighbor_nodal),
    ScalarCoupleable(moose_object),
    NeighborMooseVariableInterface(moose_object, nodal),
    MooseVariableDependencyInterface(moose_object)
  {
    addMooseVariableDependency(getCoupledMooseVars());
    addMooseVariableDependency(mooseVariable());
  }
};

#endif // NEIGHBORCOUPLEABLEMOOSEVARIABLEDEPENDENCYINTERMEDIATEINTERFACE_H
