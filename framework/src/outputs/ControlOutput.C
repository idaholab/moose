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
#include "ControlOutput.h"
#include "MooseApp.h"
#include "MooseObjectParameterName.h"
#include "InputParameterWarehouse.h"
#include "ConsoleUtils.h"

template <>
InputParameters
validParams<ControlOutput>()
{
  // Get the base class parameters
  InputParameters params = validParams<BasicOutput<Output>>();
  params.set<MultiMooseEnum>("execute_on") = "initial timestep_begin";
  params.addParam<bool>(
      "clear_after_output", true, "Clear the active control display after each output.");
  params.addParam<bool>("show_active_objects", true, "List active MooseObjects.");

  // Return the InputParameters
  return params;
}

ControlOutput::ControlOutput(const InputParameters & parameters)
  : BasicOutput<Output>(parameters),
    _clear_after_output(getParam<bool>("clear_after_output")),
    _show_active_objects(getParam<bool>("show_active_objects"))
{
}

void
ControlOutput::output(const ExecFlagType & type)
{
  if (type == EXEC_INITIAL)
    outputControls();
  else
    outputChangedControls();

  if (_show_active_objects)
    outputActiveObjects();
}

void
ControlOutput::outputActiveObjects()
{
  // Extract InputParameter objects from warehouse
  InputParameterWarehouse & wh = _app.getInputParameterWarehouse();
  const auto & params = wh.getInputParameters();

  // Populate a map based on unique InputParameter objects
  std::map<std::shared_ptr<InputParameters>, std::set<MooseObjectName>> objects;
  for (const auto & iter : params)
    objects[iter.second].insert(iter.first);

  // The stream to build
  std::stringstream oss;
  oss << std::left;

  // Loop through unique objects
  oss << "Active Objects:\n" << COLOR_DEFAULT;
  for (const auto & iter : objects)
  {
    std::shared_ptr<InputParameters> ptr = iter.first;
    if (ptr->get<bool>("enable"))
    {
      // We print slightly differently in the first iteration of the loop.
      bool first_iteration = true;
      for (const auto & obj_name : iter.second)
      {
        if (first_iteration)
        {
          oss << ConsoleUtils::indent(2) << COLOR_YELLOW << obj_name << COLOR_DEFAULT << '\n';
          first_iteration = false;
        }
        else
          oss << ConsoleUtils::indent(4) << obj_name << '\n';
      }
    }
  }

  _console << oss.str() << std::endl;
}

void
ControlOutput::outputControls()
{

  // Extract InputParameter objects from warehouse
  InputParameterWarehouse & wh = _app.getInputParameterWarehouse();
  const auto & params = wh.getInputParameters();

  // The stream to build
  std::stringstream oss;
  oss << std::left;

  // Populate a map based on unique InputParameter objects
  std::map<std::shared_ptr<InputParameters>, std::set<MooseObjectName>> objects;
  for (const auto & iter : params)
    objects[iter.second].insert(iter.first);

  // Produce the control information
  oss << "Controls:\n";
  for (const auto & iter : objects)
  {
    std::shared_ptr<InputParameters> ptr = iter.first;

    const std::set<std::string> & names = ptr->getControllableParameters();

    if (!names.empty())
    {
      oss << ConsoleUtils::indent(2) << COLOR_YELLOW << ptr->get<std::string>("_object_name")
          << COLOR_DEFAULT << '\n';

      // Full names(s)
      oss << ConsoleUtils::indent(4) << "Name(s): ";
      for (const auto & obj_name : iter.second)
        oss << obj_name << " ";
      oss << '\n';

      // Tag(s)
      const std::vector<std::string> & tags = ptr->get<std::vector<std::string>>("control_tags");
      if (!tags.empty())
      {
        oss << ConsoleUtils::indent(4) << "Tag(s): ";
        for (const auto & tag_name : tags)
          oss << tag_name << " ";
        oss << '\n';
      }

      oss << ConsoleUtils::indent(4) << "Parameter(s):\n";
      for (const auto & param_name : names)
        oss << ConsoleUtils::indent(6) << std::setw(ConsoleUtils::console_field_width) << param_name
            << ptr->type(param_name) << '\n';
    }
  }

  _console << oss.str() << std::endl;
}

void
ControlOutput::outputChangedControls()
{
  // Extract InputParameter objects from warehouse
  InputParameterWarehouse & wh = _app.getInputParameterWarehouse();
  const auto & controls = wh.getControlledParameters();

  // The stream to build
  std::stringstream oss;
  oss << std::left;

  // Print header
  if (!controls.empty())
    oss << "Active Controls:\n";

  // Loop over the controlled parameters
  for (const auto & iter : controls)
  {
    const auto ptr = iter.first;
    oss << "  " << COLOR_YELLOW << ptr->get<std::string>("_object_name") << COLOR_DEFAULT << '\n';

    // Tag(s)
    const std::vector<std::string> & tags = ptr->get<std::vector<std::string>>("control_tags");
    if (!tags.empty())
    {
      oss << ConsoleUtils::indent(4) << "Tag(s): ";
      for (const auto & tag_name : tags)
        oss << tag_name << " ";
      oss << '\n';
    }

    oss << ConsoleUtils::indent(4) << "Parameter(s):\n";
    for (const auto & param_name : iter.second)
      oss << ConsoleUtils::indent(6) << std::setw(ConsoleUtils::console_field_width)
          << param_name.parameter() << ptr->type(param_name.parameter()) << '\n';
  }

  _console << oss.str() << std::endl;

  if (_clear_after_output)
    wh.clearControlledParameters();
}
