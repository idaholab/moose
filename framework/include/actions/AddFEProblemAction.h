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

#ifndef ADDFEPROBLEM_H
#define ADDFEPROBLEM_H

#include "Action.h"

class AddFEProblemAction;

template<>
InputParameters validParams<AddFEProblemAction>();

/**
 *
 */
class AddFEProblemAction : public Action
{
public:
  AddFEProblemAction(const std::string & name, InputParameters parameters);
  virtual ~AddFEProblemAction();

  virtual void act();

protected:
  FileName _input_filename;
};

#endif /* ADDFEPROBLEM_H */
