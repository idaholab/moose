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

// Moose includes
#include "Tecplot.h"
#include "MooseApp.h"
#include "FEProblem.h"

// libMesh includes
#include "libmesh/tecplot_io.h"

template<>
InputParameters validParams<Tecplot>()
{
  // Get the base class parameters
  InputParameters params = validParams<OversampleOutputter>();

  // Supress un-available and meaningless parameters for this object
  params.suppressParameter<bool>("output_nodal_variables");
  params.suppressParameter<bool>("output_elemental_variables");
  params.suppressParameter<bool>("output_scalar_variables");
  params.suppressParameter<bool>("output_postprocessors");
  params.suppressParameter<bool>("scalar_as_nodal");
  params.suppressParameter<bool>("sequence");

  // Add binary toggle
  params.addParam<bool>("binary", false, "Set VTK files to output in binary format");
  params.addParamNamesToGroup("binary", "Advanced");

  // Add description for the Tecplot class
  params.addClassDescription("Object for outputting data in the Tecplot format");

  // Return the InputParameters
  return params;
}

Tecplot::Tecplot(const std::string & name, InputParameters parameters) :
    OversampleOutputter(name, parameters),
    _binary(getParam<bool>("binary"))
{
  // Force sequence output
  /* Note: This does not change the behavior for this object b/c outputSetup() is empty, but it is
   * place here for consistency */
  sequence(true);
}

void
Tecplot::output()
{
  TecplotIO out(*_mesh_ptr, _binary);
  out.write_equation_systems(filename(), *_es_ptr);
}

void
Tecplot::outputNodalVariables()
{
  mooseError("Individual output of nodal variables is not support for Tecplot output");
}

void
Tecplot::outputElementalVariables()
{
  mooseError("Individual output of elemental variables is not support for Tecplot output");
}

void
Tecplot::outputPostprocessors()
{
  mooseError("Individual output of postprocessors is not support for Tecplot output");
}

void
Tecplot::outputScalarVariables()
{
  mooseError("Individual output of scalars is not support for Tecplot output");
}

std::string
Tecplot::filename()
{
  // Append the padded time step to the file base
  std::ostringstream output;
  output << _file_base
         << "_"
         << std::setw(_padding)
         << std::setprecision(0)
         << std::setfill('0')
         << std::right
         << _t_step;
  return output.str() + ".plt";
}
