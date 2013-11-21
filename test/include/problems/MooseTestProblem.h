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

#ifndef MOOSETESTPROBLEM_H
#define MOOSETESTPROBLEM_H

#include "FEProblem.h"

class MooseTestProblem;

template<>
InputParameters validParams<MooseTestProblem>();

/**
 * FEProblem derived class for customization of callbacks. In this instance we only print out something in the c-tor and d-tor, so we know the class was build and used properly.
 */
class MooseTestProblem : public FEProblem
{
public:
  MooseTestProblem(const std::string & name, InputParameters params);
  virtual ~MooseTestProblem();
};

#endif /* MOOSETESTPROBLEM_H */
