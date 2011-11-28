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

#include "OnOffNeumannBC.h"

template<>
InputParameters validParams<OnOffNeumannBC>()
{
  InputParameters params = validParams<NeumannBC>();

  return params;
}

OnOffNeumannBC::OnOffNeumannBC(const std::string & name, InputParameters parameters) :
    NeumannBC(name, parameters)
{
}

bool
OnOffNeumannBC::shouldApply()
{
  return (_t_step == 1) ? false : true;
}
