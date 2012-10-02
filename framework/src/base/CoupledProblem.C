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

#include "CoupledProblem.h"

template<>
InputParameters validParams<CoupledProblem>()
{
  InputParameters params = validParams<Problem>();
  return params;
}


CoupledProblem::CoupledProblem(const std::string & name, InputParameters params) :
    Problem(name, params)
{
}

CoupledProblem::~CoupledProblem()
{
}

void
CoupledProblem::init()
{
}

void
CoupledProblem::output(bool /*force*/)
{
}
