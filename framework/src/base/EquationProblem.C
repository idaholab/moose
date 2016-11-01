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

#include "EquationProblem.h"

template<>
InputParameters validParams<EquationProblem>()
{
  InputParameters params = validParams<FEProblem>();
  params.addParam<bool>("use_nonlinear", true, "Determines whether to use a Nonlinear vs a Eigenvalue system (Automatically determined based on executioner)");
  return params;
}

EquationProblem::EquationProblem(const InputParameters & parameters) :
    FEProblem(parameters)
{

}

EquationProblem::~EquationProblem()
{

}
