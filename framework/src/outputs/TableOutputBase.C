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
#include "TableOutputBase.h"
#include "FEProblem.h"
#include "Postprocessor.h"
#include "PetscSupport.h"
#include "Executioner.h"
#include "MooseApp.h"
#include "Conversion.h"

// libMesh includes
#include "libmesh/string_to_enum.h"

template<>
InputParameters validParams<TableOutputBase>()
{
  MooseEnum pps_fit_mode(FormattedTable::getWidthModes());

  InputParameters params = validParams<OutputBase>();

  // Suppressing the output of nodal and elemental variables disables this type of output
  params.suppressParameter<bool>("output_elemental_variables");
  params.suppressParameter<bool>("output_nodal_variables");
  params.suppressParameter<bool>("elemental_as_nodal");
  params.suppressParameter<bool>("scalar_as_nodal");

  // This is not used currently, so suppress it
  params.suppressParameter<bool>("output_input");

  return params;
}

TableOutputBase::TableOutputBase(const std::string & name, InputParameters parameters) :
    OutputBase(name, parameters)
{
}

TableOutputBase::~TableOutputBase()
{
}

void
TableOutputBase::outputNodalVariables()
{
  mooseError("Nodal nonlinear variable output not supported by TableOutputBase output class");
}

void
TableOutputBase::outputElementalVariables()
{
  mooseError("Elemental nonlinear variable output not supported by TableOutputBase output class");
}

void
TableOutputBase::outputPostprocessors()
{
  // List of names of the postprocessors to output
  const std::vector<std::string> & out = getPostprocessorOutput();

  // Loop through the postprocessor names and extract the values from the PostprocessorData storage
  for (std::vector<std::string>::const_iterator it = out.begin(); it != out.end(); ++it)
  {
    PostprocessorValue value = _problem_ptr->getPostprocessorValue(*it);
    _postprocessor_table.addData(*it, value, _time);
    _all_data_table.addData(*it, value, _time);
  }
}

void
TableOutputBase::outputScalarVariables()
{
  // List of scalar variables
  const std::vector<std::string> & out = getScalarOutput();

  // Loop through each variable
  for (std::vector<std::string>::const_iterator it = out.begin(); it != out.end(); ++it)
  {
    // Get reference to the variable and the no. of components
    VariableValue & variable = _problem_ptr->getScalarVariable(0, *it).sln();
    unsigned int n = variable.size();

    // If the variable has a single component, simply output the value with the name
    if (n == 1)
    {
      _scalar_table.addData(*it, variable[0], _time);
      _all_data_table.addData(*it, variable[0], _time);
    }

    // Multi-component variables are appened with the component index
    else
      for (unsigned int i = 0; i < n; ++i)
      {
        std::ostringstream os;
        os << *it << "_" << i;
        _scalar_table.addData(os.str(), variable[i], _time);
        _all_data_table.addData(os.str(), variable[i], _time);
      }
  }
}
