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
#include "Exodus.h"
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

AddOutputAction::AddOutputAction(const std::string & name, InputParameters params) :
    MooseObjectAction(name, params)
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
  std::string object_name = getShortName();

  // Reject the reserved names for objects not built by MOOSE
  if (!_moose_object_pars.get<bool>("_built_by_moose") && output_warehouse.isReservedName(object_name))
    mooseError("The name '" << object_name << "' is a reserved name for output objects");

  // Check that an object by the same name does not already exist; this must be done before the object
  // is created to avoid getting misleading errors from the Parser
  if (output_warehouse.hasOutput(object_name))
    mooseError("The output object named '" << object_name << "' already exists");

  // Add a pointer to the FEProblem class
  _moose_object_pars.addPrivateParam<FEProblem *>("_fe_problem",  _problem.get());

  // Create common parameter exclude list
  std::vector<std::string> exclude;
  if (_type == "Console")
    exclude.push_back("output_on");

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

  // If creating an Exodus output and "ensight_time" is enabled, create a postprocessor for reporting the time
  MooseSharedPointer<Exodus> exodus = MooseSharedNamespace::dynamic_pointer_cast<Exodus>(output);
  if (exodus && exodus->getParam<bool>("ensight_time"))
  {
    InputParameters params = _factory.getValidParams("TimePostprocessor");
    params.set<MultiMooseEnum>("execut_on") = "initial timestep_end";
    _problem->addPostprocessor("TimePostprocessor", "simulation_time", params);
  }
}
