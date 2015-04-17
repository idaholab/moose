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
#include "CheckOutputAction.h"
#include "Material.h"
#include "MooseApp.h"
#include "Console.h"
#include "CommonOutputAction.h"
#include "AddVariableAction.h"

template<>
InputParameters validParams<CheckOutputAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}

CheckOutputAction::CheckOutputAction(const std::string & name, InputParameters params) :
  Action(name, params)
{
}

CheckOutputAction::~CheckOutputAction()
{
}

void
CheckOutputAction::act()
{
  // Perform the various output related checks
  checkVariableOutput("add_variable");
  checkVariableOutput("add_aux_variable");
  checkMaterialOutput();
  checkConsoleOutput();
  checkPerfLogOutput();
  checkInputOutput();
}

void
CheckOutputAction::checkVariableOutput(const std::string & task)
{
  if (_awh.hasActions(task))
  {
    // Loop through the actions for the given task
    const std::vector<Action *> & actions = _awh.getActionsByName(task);
    for (std::vector<Action *>::const_iterator it = actions.begin(); it != actions.end(); ++it)
    {
      // Cast the object to AddVariableAction so that that OutputInterface::buildOutputHideVariableList may be called
      AddVariableAction * ptr = dynamic_cast<AddVariableAction*>(*it);

      // If the cast fails move to the next action, this is the case with NodalNormals which is also associated with
      // the "add_aux_variable" task.
      if (ptr == NULL)
        continue;

      // Create the hide list for the action
      std::set<std::string> names_set;
      names_set.insert(ptr->getShortName());
      ptr->buildOutputHideVariableList(names_set);
    }
  }
}

void
CheckOutputAction::checkMaterialOutput()
{
  // Do nothing if _problem is NULL (this is the case for coupled problems)
  /* Do not produce warning, you will get a warning from OutputAction */
  if (_problem.get() == NULL)
    return;

  // A complete list of all Material objects
  std::vector<Material *> materials = _problem->getMaterialWarehouse(0).all();

  // Loop through each material object
  for (std::vector<Material *>::iterator material_iter = materials.begin(); material_iter != materials.end(); ++material_iter)
  {
    // Extract the names of the output objects to which the material properties will be exported
    std::set<OutputName> outputs = (*material_iter)->getOutputs();

    // Check that the outputs exist
    _app.getOutputWarehouse().checkOutputs(outputs);
  }
}

void
CheckOutputAction::checkConsoleOutput()
{
  // Warning if multiple Console objects are added with 'output_screen=true' in the input file
  std::vector<Console *> console_ptrs = _app.getOutputWarehouse().getOutputs<Console>();
  unsigned int num_screen_outputs = 0;
  for (std::vector<Console *>::iterator it = console_ptrs.begin(); it != console_ptrs.end(); ++it)
    if ((*it)->getParam<bool>("output_screen"))
      num_screen_outputs++;

  if (num_screen_outputs > 1)
    mooseWarning("Multiple (" << num_screen_outputs << ") Console output objects are writing to the screen, this will likely cause duplicate messages printed.");
}

void
CheckOutputAction::checkPerfLogOutput()
{

  // Search for the existence of a Console output object
  bool has_console = false;
  std::vector<Console *> ptrs = _app.getOutputWarehouse().getOutputs<Console>();
  for (std::vector<Console *>::const_iterator it = ptrs.begin(); it != ptrs.end(); ++it)
    if ((*it)->getParam<bool>("output_screen"))
    {
      has_console = true;
      break;
    }

  /* If a Console outputter is found then all the correct handling of performance logs are
     handled within the object(s), so do nothing */
  if (!has_console)
  {
    Moose::perf_log.disable_logging();
    Moose::setup_perf_log.disable_logging();
#ifdef LIBMESH_ENABLE_PERFORMANCE_LOGGING
    libMesh::perflog.disable_logging();
#endif
  }

  // If the --timing option is used from the command-line, enable all logging
  if (_app.getParam<bool>("timing"))
  {
    Moose::perf_log.enable_logging();
    Moose::setup_perf_log.enable_logging();
#ifdef LIBMESH_ENABLE_PERFORMANCE_LOGGING
    libMesh::perflog.enable_logging();
#endif
  }
}

void
CheckOutputAction::checkInputOutput()
{
  if (_app.getParam<bool>("show_input"))
  {
    const std::vector<Console *> ptrs = _app.getOutputWarehouse().getOutputs<Console>();
    for (std::vector<Console *>::const_iterator it = ptrs.begin(); it != ptrs.end(); ++it)
      if ((*it)->getParam<bool>("output_screen"))
        (*it)->parameters().set<MultiMooseEnum>("output_input_on") = "initial"; // Output::getExecuteOptions("initial");
  }
}
