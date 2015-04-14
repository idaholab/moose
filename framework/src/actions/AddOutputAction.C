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
#include "AddOutputAction.h"
#include "FEProblem.h"
#include "Factory.h"
#include "OutputWarehouse.h"
#include "Output.h"
#include "MooseApp.h"
#include "FileMesh.h"
#include "MooseApp.h"

// libMesh includes
#include "libmesh/mesh_function.h"
#include "libmesh/mesh_refinement.h"
#include "libmesh/explicit_system.h"

template<>
InputParameters validParams<AddOutputAction>()
{
   InputParameters params = validParams<MooseObjectAction>();
   return params;
}

AddOutputAction::AddOutputAction(InputParameters params) :
    MooseObjectAction(params)
{
}

void
AddOutputAction::act()
{
  // Do nothing if FEProblem is NULL, this should only be the case for CoupledProblem
  if (_problem.get() == NULL)
    return;

  // Get a reference to the OutputWarehouse
  OutputWarehouse & output_warehouse = _app.getOutputWarehouse();

  // Get the output object name
  std::string object_name = _name;

  // Reject the reserved names for objects not built by MOOSE
  std::string short_name = MooseUtils::shortName(object_name);
  if (!_moose_object_pars.get<bool>("_built_by_moose") && output_warehouse.isReservedName(short_name))
    mooseError("The name '" << short_name << "' is a reserved name for output objects");

  // Check that an object by the same name does not already exist; this must be done before the object
  // is created to avoid getting misleading errors from the Parser
  if (output_warehouse.hasOutput(object_name))
    mooseError("The output object named '" << object_name << "' already exists");

  // Add a pointer to the FEProblem class
  _moose_object_pars.addPrivateParam<FEProblem *>("_fe_problem",  _problem.get());

  // Create common parameter exclude list
  std::vector<std::string> exclude;
  if (_type == "Console")
  {
    exclude.push_back("output_on");

    // --show-input should enable the display of the input file on the screen
    if (_app.getParam<bool>("show_input") && _moose_object_pars.get<bool>("output_screen"))
      _moose_object_pars.set<MultiMooseEnum>("output_input_on") = "initial";
  }

  // Apply the common parameters
  InputParameters * common = output_warehouse.getCommonParameters();
  if (common != NULL)
    _moose_object_pars.applyParameters(*common, exclude);

  // Set the correct value for the binary flag for XDA/XDR output
  if (_type == "XDR")
    _moose_object_pars.set<bool>("_binary") = true;
  else if (_type == "XDA")
    _moose_object_pars.set<bool>("_binary") = false;

  // Adjust the checkpoint suffix if auto recovery was enabled
  if (object_name == "auto_recovery_checkpoint")
    _moose_object_pars.set<std::string>("suffix") = "auto_recovery";

  // Create the object and add it to the warehouse
  MooseSharedPointer<Output> output = MooseSharedNamespace::static_pointer_cast<Output>(_factory.create(_type, object_name, _moose_object_pars));
  output_warehouse.addOutput(output);
}


// DEPRECATED CONSTRUCTOR
AddOutputAction::AddOutputAction(const std::string & deprecated_name, InputParameters params) :
    MooseObjectAction(deprecated_name, params)
{
}
