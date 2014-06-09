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
#include "NonlinearSystem.h"
#include "FEProblem.h"

template<>
InputParameters validParams<SolutionHistory>()
{
  // Get the parameters from the parent object
  InputParameters params = validParams<FileOutput>();

  // Suppress unused parameters
  params.suppressParameter<unsigned int>("padding");
  params.suppressParameter<bool>("output_nodal_variables");
  params.suppressParameter<bool>("output_elemental_variables");
  params.suppressParameter<bool>("output_scalar_variables");
  params.suppressParameter<bool>("output_postprocessors");
  params.suppressParameter<bool>("output_vector_postprocessors");
  params.suppressParameter<bool>("output_input");

  // Return the parameters
  return params;

}

SolutionHistory::SolutionHistory(const std::string & name, InputParameters & parameters) :
    FileOutput(name, parameters)
{
}

SolutionHistory::~SolutionHistory()
{
}

std::string
SolutionHistory::filename()
{
  return _file_base + ".slh";
}

void
SolutionHistory::output()
{
  // Reference to the Non-linear System
  NonlinearSystem & nl_sys = _problem_ptr->getNonlinearSystem();

  std::ofstream slh_file;
  slh_file.open(filename().c_str(), std::ios::app);
  slh_file << nl_sys._current_nl_its;

  for (unsigned int i = 0; i < nl_sys._current_l_its.size(); i++)
    slh_file << " " << nl_sys._current_l_its[i];

  slh_file << std::endl;
}

void
SolutionHistory::outputNodalVariables()
{
  mooseError("Individual output of nodal variables is not supported for solution hisotry output");
}

void
SolutionHistory::outputElementalVariables()
{
  mooseError("Individual output of elemental variables is not supported for solution history output");
}

void
SolutionHistory::outputPostprocessors()
{
  mooseError("Individual output of postprocessors is not supported for solution history output");
}

void
SolutionHistory::outputVectorPostprocessors()
{
  mooseError("Individual output of VectorPostprocessors is not supported for solution history output");
}

void
SolutionHistory::outputScalarVariables()
{
  mooseError("Individual output of scalars is not supported for solution history output");
}
