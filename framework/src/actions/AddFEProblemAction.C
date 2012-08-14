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

#include "AddFEProblemAction.h"
#include "CoupledExecutioner.h"

template<>
InputParameters validParams<AddFEProblemAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<FileName>("input_file", "File name of the input file");
  return params;
}

AddFEProblemAction::AddFEProblemAction(const std::string & name, InputParameters parameters) :
    Action(name, parameters),
    _input_filename(getParam<FileName>("input_file"))
{
}

AddFEProblemAction::~AddFEProblemAction()
{
}

void
AddFEProblemAction::act()
{
  CoupledExecutioner * master_executioner = dynamic_cast<CoupledExecutioner *>(_executioner);
  if (master_executioner != NULL)
  {
    master_executioner->addFEProblem(getShortName(), _input_filename);
  }
}
