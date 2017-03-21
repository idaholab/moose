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
#include "XDA.h"
#include "MooseApp.h"
#include "FEProblem.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<XDA>()
{
  // Get the base class parameters
  InputParameters params = validParams<BasicOutput<OversampleOutput>>();

  // Add description for the XDA class
  params.addClassDescription("Object for outputting data in the XDA/XDR format");

  /* Set a private parameter for controlling the output type (XDR = binary), the value
     of this parameter is set by the AddOutputAction*/
  params.addPrivateParam<bool>("_binary", false);

  // Return the InputParameters
  return params;
}

XDA::XDA(const InputParameters & parameters)
  : BasicOutput<OversampleOutput>(parameters), _binary(getParam<bool>("_binary"))
{
}

void
XDA::output(const ExecFlagType & /*type*/)
{
  // Strings for the two filenames to be written
  std::string es_name = filename();
  std::string mesh_name = es_name;

  // Make sure the filename has an extension
  if (es_name.size() < 4)
    mooseError("Unacceptable filename, you must include an extension (.xda or .xdr).");

  // Insert the mesh suffix
  mesh_name.insert(mesh_name.size() - 4, "_mesh");

  // Set the binary flag
  XdrMODE mode = _binary ? ENCODE : WRITE;

  // Write the files
  _mesh_ptr->getMesh().write(mesh_name);
  _es_ptr->write(
      es_name, mode, EquationSystems::WRITE_DATA | EquationSystems::WRITE_ADDITIONAL_DATA);
  _file_num++;
}

std::string
XDA::filename()
{
  // Append the padded time step to the file base
  std::ostringstream output;
  output << _file_base << "_" << std::setw(_padding) << std::setprecision(0) << std::setfill('0')
         << std::right << _file_num;

  if (_binary)
    output << ".xdr";
  else
    output << ".xda";
  return output.str();
}
