//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PPBindingSteady.h"
#include "NonlinearSystem.h"
#include "AuxiliarySystem.h"

registerMooseObject("MooseTestApp", PPBindingSteady);

InputParameters
PPBindingSteady::validParams()
{
  InputParameters params = Steady::validParams();
  params.addParam<PostprocessorName>(
      "postprocessor", 1, "A postprocessor to be used by this executioner");
  return params;
}

PPBindingSteady::PPBindingSteady(const InputParameters & parameters)
  : Steady(parameters),
    _pp(getPostprocessorValue("postprocessor")),
    _pp_old(getPostprocessorValueOld("postprocessor")),
    _pp_older(getPostprocessorValueOlder("postprocessor"))
{
}

void
PPBindingSteady::init()
{
  Steady::init();
  const PostprocessorName & pp_name = getParam<PostprocessorName>("postprocessor");

  // check if the references obtained during construction point to the right data late in init
  if (((&_pp) != &_problem.getPostprocessorValueByName(pp_name)) ||
      ((&_pp_old) != &_problem.getPostprocessorValueByName(pp_name, 1)) ||
      ((&_pp_older) != &_problem.getPostprocessorValueByName(pp_name, 2)))
    mooseError("Postprocessor binding in executioner is wrong");
  else
    _console << "Postprocessor binding is OK" << std::endl;
}
