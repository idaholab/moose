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

#ifndef COUPLEABLEMOOSEVARIABLEDEPENDENCYINTERMEDIATEINTERFACE_H
#define COUPLEABLEMOOSEVARIABLEDEPENDENCYINTERMEDIATEINTERFACE_H

#include "Coupleable.h"
#include "MooseVariableInterface.h"
#include "MooseVariableDependencyInterface.h"

/**
 * Intermediate base class that ties together all the interfaces for getting
 * MooseVariables with the MooseVariableDependencyInterface
 */
class CoupleableMooseVariableDependencyIntermediateInterface :
  public Coupleable,
  public ScalarCoupleable,
  public MooseVariableInterface,
  public MooseVariableDependencyInterface
{
public:
  CoupleableMooseVariableDependencyIntermediateInterface(InputParameters & parameters, bool nodal) :
    Coupleable(parameters, nodal),
    ScalarCoupleable(parameters),
    MooseVariableInterface(parameters, nodal)
  {
    const std::vector<MooseVariable *> & coupled_vars = getCoupledMooseVars();
    for(unsigned int i=0; i<coupled_vars.size(); i++)
      addMooseVariableDependency(coupled_vars[i]);

    addMooseVariableDependency(mooseVariable());
  }
};

#endif // COUPLEABLEMOOSEVARIABLEDEPENDENCYINTERMEDIATEINTERFACE_H
