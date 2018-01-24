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
