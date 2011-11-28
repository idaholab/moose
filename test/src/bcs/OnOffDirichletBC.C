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

#include "OnOffDirichletBC.h"

template<>
InputParameters validParams<OnOffDirichletBC>()
{
  InputParameters params = validParams<DirichletBC>();

  return params;
}

OnOffDirichletBC::OnOffDirichletBC(const std::string & name, InputParameters parameters) :
    DirichletBC(name, parameters)
{
}

OnOffDirichletBC::~OnOffDirichletBC()
{
}

bool
OnOffDirichletBC::shouldApply()
{
  return (_t_step == 1) ? true : false;
}
