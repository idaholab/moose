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

#include "NodalBCBase.h"

template<>
InputParameters validParams<NodalBCBase>()
{
  InputParameters params = validParams<BoundaryCondition>();
  params += validParams<RandomInterface>();

  params.addParam<std::vector<AuxVariableName> >("save_in", "The name of auxiliary variables to save this BC's residual contributions to.  Everything about that variable must match everything about this variable (the type, what blocks it's on, etc.)");
  params.addParam<std::vector<AuxVariableName> >("diag_save_in", "The name of auxiliary variables to save this BC's diagonal jacobian contributions to.  Everything about that variable must match everything about this variable (the type, what blocks it's on, etc.)");

  return params;
}

NodalBCBase::NodalBCBase(const InputParameters & parameters) :
    BoundaryCondition(parameters, true), // true is for being Nodal
    RandomInterface(parameters, _fe_problem, _tid, true),
    CoupleableMooseVariableDependencyIntermediateInterface(this, true)
{
}
