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

// MOOSE includes
#include "SolutionHistory.h"
#include "NonlinearSystemBase.h"
#include "FEProblem.h"

template <>
InputParameters
validParams<SolutionHistory>()
{
  // Get the parameters from the parent object
  InputParameters params = validParams<BasicOutput<FileOutput>>();

  // Return the parameters
  return params;
}

SolutionHistory::SolutionHistory(const InputParameters & parameters)
  : BasicOutput<FileOutput>(parameters)
{
}

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
  slh_file << nl_sys._current_nl_its;

  for (const auto & linear_its : nl_sys._current_l_its)
    slh_file << " " << linear_its;

  slh_file << std::endl;
}
