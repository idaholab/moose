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
#include "XDA.h"
#include "MooseApp.h"
#include "FEProblem.h"

template<>
InputParameters validParams<XDA>()
{
  // Get the base class parameters

  InputParameters params = validParams<OversampleOutputter>();
  params += validParams<FileOutputInterface>();

  // Supress un-available parameters
  params.suppressParameter<bool>("output_nodal_variables");
  params.suppressParameter<bool>("output_elemental_variables");
  params.suppressParameter<bool>("output_scalar_variables");
  params.suppressParameter<bool>("output_postprocessors");
  params.suppressParameter<bool>("scalar_as_nodal");
  params.suppressParameter<bool>("sequence");

  // Add description for the XDA class
  params.addClassDescription("Object for outputting data in the XDA/XDR format");

  /* Set a private parameter for controlling the output type (XDR = binary), the value
     of this parameter is set by the AddOutputAction*/
  params.addPrivateParam<bool>("_binary", false);

  // Return the InputParameters
  return params;
}

XDA::XDA(const std::string & name, InputParameters parameters) :
    OversampleOutputter(name, parameters),
    FileOutputInterface(name, parameters),
    _binary(getParam<bool>("_binary"))
{
  // Force sequence output
  /* Note: This does not change the behavior for this object b/c outputSetup() is empty, but it is
   * place here for consistency */
  sequence(true);
}

void
XDA::output()
{
  if (_binary)
  {
    _mesh_ptr->getMesh().write(filename()+"_mesh.xdr");
    _es_ptr->write (filename()+".xdr", ENCODE, EquationSystems::WRITE_DATA | EquationSystems::WRITE_ADDITIONAL_DATA);
  }
  else
  {
    _mesh_ptr->getMesh().write(filename()+"_mesh.xda");
    _es_ptr->write (filename()+".xda", WRITE, EquationSystems::WRITE_DATA | EquationSystems::WRITE_ADDITIONAL_DATA);
  }
}

void
XDA::outputNodalVariables()
{
  mooseError("Individual output of nodal variables is not support for XDA/XDR output");
}

void
XDA::outputElementalVariables()
{
  mooseError("Individual output of elemental variables is not support for XDA/XDR output");
}

void
XDA::outputPostprocessors()
{
  mooseError("Individual output of postprocessors is not support for XDA/XDR output");
}

void
XDA::outputScalarVariables()
{
  mooseError("Individual output of scalars is not support for XDA/XDR output");
}

std::string
XDA::filename()
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
  return output.str();
}
