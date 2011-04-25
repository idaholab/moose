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

#include "AddPostprocessorAction.h"
#include "Factory.h"
#include "Parser.h"
#include "MProblem.h"

template<>
InputParameters validParams<AddPostprocessorAction>()
{
  InputParameters params = validParams<MooseObjectAction>();
  return params;
}

AddPostprocessorAction::AddPostprocessorAction(const std::string & name, InputParameters params) :
    MooseObjectAction(name, params)
{
  if (_parser_handle.pathContains(_name, "Residual"))
    _pps_type = EXEC_RESIDUAL;
  else if (_parser_handle.pathContains(_name, "Jacobian"))
    _pps_type = EXEC_JACOBIAN;
  else
    _pps_type = EXEC_TIMESTEP;
}

void
AddPostprocessorAction::act() 
{
  _parser_handle._problem->addPostprocessor(_type, getShortName(), _moose_object_pars, _pps_type);
}
