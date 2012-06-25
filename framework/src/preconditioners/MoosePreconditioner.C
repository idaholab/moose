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

#include "MoosePreconditioner.h"
#include "FEProblem.h"

template<>
InputParameters validParams<MoosePreconditioner>()
{
  InputParameters params = validParams<MooseObject>();
  params.addPrivateParam<FEProblem *>("_fe_problem");
  params.addPrivateParam<std::string>("built_by_action", "add_preconditioning");
  return params;
}


MoosePreconditioner::MoosePreconditioner(const std::string & name, InputParameters params) :
    MooseObject(name, params),
    _fe_problem(*getParam<FEProblem *>("_fe_problem"))
{
}

MoosePreconditioner::~MoosePreconditioner()
{
}

