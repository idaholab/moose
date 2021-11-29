//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "SolutionHistory.h"
#include "NonlinearSystemBase.h"
#include "FEProblem.h"

#include <fstream>

registerMooseObject("MooseApp", SolutionHistory);

InputParameters
SolutionHistory::validParams()
{
  // Get the parameters from the parent object
  InputParameters params = FileOutput::validParams();
  params.addClassDescription("Outputs the non-linear and linear iteration solve history.");

  // Return the parameters
  return params;
}

SolutionHistory::SolutionHistory(const InputParameters & parameters) : FileOutput(parameters) {}

std::string
SolutionHistory::filename()
{
  return _file_base + ".slh";
}

void
SolutionHistory::output(const ExecFlagType & /*type*/)
{
  // Reference to the Non-linear System
  NonlinearSystemBase & nl_sys = _problem_ptr->getNonlinearSystemBase();

  std::ofstream slh_file;
  slh_file.open(filename().c_str(), std::ios::app);
  if (slh_file.fail())
    mooseError("Unable to open file ", filename());

  slh_file << nl_sys._current_nl_its;

  for (const auto & linear_its : nl_sys._current_l_its)
    slh_file << " " << linear_its;

  slh_file << std::endl;
}
