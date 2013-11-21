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

#include "MooseTestProblem.h"

template<>
InputParameters validParams<MooseTestProblem>()
{
  InputParameters params = validParams<FEProblem>();
  return params;
}


MooseTestProblem::MooseTestProblem(const std::string & name, InputParameters params) :
    FEProblem(name, params)
{
  Moose::out << "Hello, I am your FEProblem-derived class and my name is '" << this->name() << "'" << std::endl;
}

MooseTestProblem::~MooseTestProblem()
{
  Moose::out << "Goodbye!" << std::endl;
}

