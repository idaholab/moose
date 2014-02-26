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

#include "AddOutputAction.h"
#include "FEProblem.h"
#include "Factory.h"
#include "OutputWarehouse.h"
#include "OutputBase.h"
#include "MooseApp.h"
#include "FileMesh.h"
#include "MooseApp.h"

#include "libmesh/mesh_function.h"
#include "libmesh/mesh_refinement.h"
#include "libmesh/explicit_system.h"

template<>
InputParameters validParams<AddOutputAction>()
{
   InputParameters params = validParams<MooseObjectAction>();
   return params;
}

AddOutputAction::AddOutputAction(const std::string & name, InputParameters params) :
    MooseObjectAction(name, params),
    _output_warehouse(_app.getOutputWarehouse())
{
}

void
AddOutputAction::act()
{
  // Get the output object name
  std::string object_name = getShortName();

  // Check that an object by the same name does not already exist; this must be done before the object
  // is created to avoid getting misleading errors from the Parser
  if (_output_warehouse.hasOutput(object_name))
    mooseError("The output object named '" << object_name << "' already exists");

  // Add a pointer to the FEProblem class
  _moose_object_pars.addPrivateParam<FEProblem *>("_fe_problem",  _problem);

  // Apply the common parameters
  _moose_object_pars.applyParameters(_output_warehouse.getCommonParameters());

  // Set the file base, if it has not been set already
  if (!_moose_object_pars.isParamValid("file_base"))
    _moose_object_pars.set<std::string>("file_base") = getDefaultOutFileBase();

  // Set the correct value for the binary flag for XDA/XDR output
  if (_type.compare("XDR") == 0)
    _moose_object_pars.set<bool>("_binary") = true;
  else if (_type.compare("XDA") == 0)
    _moose_object_pars.set<bool>("_binary") = false;

  // Create the object and add it to the warehouse
  OutputBase * output = static_cast<OutputBase *>(_factory.create(_type, object_name, _moose_object_pars));
  _output_warehouse.addOutput(output);
}

std::string
AddOutputAction::getDefaultOutFileBase()
{
  std::string input_file_name = _app.getFileName();
  mooseAssert(input_file_name != "", "Input Filename is NULL");
  size_t pos = input_file_name.find_last_of('.');
  mooseAssert(pos != std::string::npos, "Unable to determine suffix of input file name");
  return input_file_name.substr(0,pos) + "_out";
}
