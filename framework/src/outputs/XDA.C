//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "XDA.h"
#include "MooseApp.h"
#include "FEProblem.h"
#include "MooseMesh.h"

// libMesh includes
#include "libmesh/enum_xdr_mode.h"

using namespace libMesh;

registerMooseObject("MooseApp", XDA);
registerMooseObjectAliased("MooseApp", XDA, "XDR");

InputParameters
XDA::validParams()
{
  // Get the base class parameters
  InputParameters params = OversampleOutput::validParams();

  // Add description for the XDA class
  params.addClassDescription("Object for outputting data in the XDA/XDR format.");

  /* Set a private parameter for controlling the output type (XDR = binary), the value
     of this parameter is set by the AddOutputAction*/
  params.addPrivateParam<bool>("_binary", false);

  // Return the InputParameters
  return params;
}

XDA::XDA(const InputParameters & parameters)
  : OversampleOutput(parameters), _binary(getParam<bool>("_binary"))
{
}

void
XDA::output()
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
