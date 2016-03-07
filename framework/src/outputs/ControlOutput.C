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

template<>
InputParameters validParams<ControlOutput>()
{
  // Get the base class parameters
  InputParameters params = validParams<BasicOutput<Output> >();
  params.set<MultiMooseEnum>("execute_on") = "initial timestep_begin";
  params.addParam<bool>("clear_after_output", true, "Clear the active control display after each output.");
  params.addParam<bool>("show_active_objects", true, "List active MooseObjects.");

  // Return the InputParameters
  return params;
}


ControlOutput::ControlOutput(const InputParameters & parameters) :
    BasicOutput<Output>(parameters),
    _clear_after_output(getParam<bool>("clear_after_output")),
    _show_active_objects(getParam<bool>("show_active_objects"))
{
}


void
ControlOutput::output(const ExecFlagType & type)
{
  switch (type)
  {
  case EXEC_INITIAL:
    outputControls();
    break;
  default:
    outputChangedControls();
  }

  if (_show_active_objects)
    outputActiveObjects();
}


void
ControlOutput::outputActiveObjects()
{
  // Extract InputParameter objects from warehouse
  InputParameterWarehouse & wh = _app.getInputParameterWarehouse();
  const std::multimap<MooseObjectName, MooseSharedPointer<InputParameters> > & params = wh.getInputParameters();

  // Populate a map based on unique InputParameter objects
  std::map<MooseSharedPointer<InputParameters>, std::set<MooseObjectName> > objects;
  for (std::multimap<MooseObjectName, MooseSharedPointer<InputParameters> >::const_iterator iter = params.begin(); iter != params.end(); ++iter)
    objects[iter->second].insert(iter->first);

  // The stream to build
  std::stringstream oss;
  oss << std::left;

  // Loop through unique objects
  oss << "Active Objects:\n" << COLOR_DEFAULT;
  for (std::map<MooseSharedPointer<InputParameters>, std::set<MooseObjectName> >::const_iterator iter = objects.begin(); iter != objects.end(); ++iter)
  {
    MooseSharedPointer<InputParameters> ptr = iter->first;
    if (ptr->get<bool>("enable"))
    {
      for (std::set<MooseObjectName>::const_iterator it = iter->second.begin(); it != iter->second.end(); ++it)
      {
        if (it == iter->second.begin())
          oss << ConsoleUtils::indent(2) << COLOR_YELLOW << *it << COLOR_DEFAULT << '\n';
        else
          oss << ConsoleUtils::indent(4) << *it << '\n';
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
  const std::multimap<MooseObjectName, MooseSharedPointer<InputParameters> > & params = wh.getInputParameters();

  // The stream to build
  std::stringstream oss;
  oss << std::left;

  // Populate a map based on unique InputParameter objects
  std::map<MooseSharedPointer<InputParameters>, std::set<MooseObjectName> > objects;
  for (std::multimap<MooseObjectName, MooseSharedPointer<InputParameters> >::const_iterator iter = params.begin(); iter != params.end(); ++iter)
    objects[iter->second].insert(iter->first);

  // Produce the control information
  oss << "Controls:\n";
  for (std::map<MooseSharedPointer<InputParameters>, std::set<MooseObjectName> >::const_iterator iter = objects.begin(); iter != objects.end(); ++iter)
  {
    MooseSharedPointer<InputParameters> ptr = iter->first;

    const std::set<std::string> & names = ptr->getControllableParameters();

    if (!names.empty())
    {
      oss << ConsoleUtils::indent(2) << COLOR_YELLOW << ptr->get<std::string>("_object_name") << COLOR_DEFAULT << '\n';

      // Full names(s)
      oss << ConsoleUtils::indent(4) << "Name(s): ";
      for (std::set<MooseObjectName>::const_iterator it = iter->second.begin(); it != iter->second.end(); ++it)
        oss << (*it) << " ";
      oss << '\n';

      // Tag(s)
      const std::vector<std::string> & tags = ptr->get<std::vector<std::string> >("control_tags");
      if (!tags.empty())
      {
        oss << ConsoleUtils::indent(4) << "Tag(s): ";
        for (std::vector<std::string>::const_iterator it = tags.begin(); it != tags.end(); ++it)
          oss << *it << " ";
        oss << '\n';
      }

      oss <<  ConsoleUtils::indent(4) << "Parameter(s):\n";
      for (std::set<std::string>::const_iterator it = names.begin(); it != names.end(); ++it)
        oss << ConsoleUtils::indent(6) << std::setw(ConsoleUtils::console_field_width) << *it << ptr->type(*it) << '\n';
    }
  }

  _console << oss.str() << std::endl;
}


void
ControlOutput::outputChangedControls()
{
  // Extract InputParameter objects from warehouse
  InputParameterWarehouse & wh = _app.getInputParameterWarehouse();
  const std::map<MooseSharedPointer<InputParameters>, std::set<MooseObjectParameterName> > & controls = wh.getControlledParameters();

  // The stream to build
  std::stringstream oss;
  oss << std::left;

  // Print header
  if (!controls.empty())
    oss << "Active Controls:\n";

  // Loop over the controlled parameters
  for (std::map<MooseSharedPointer<InputParameters>, std::set<MooseObjectParameterName> >::const_iterator iter = controls.begin(); iter != controls.end(); ++iter)
  {
    const MooseSharedPointer<InputParameters> ptr = iter->first;
    oss << "  " << COLOR_YELLOW << ptr->get<std::string>("_object_name") << COLOR_DEFAULT << '\n';

    // Tag(s)
    const std::vector<std::string> & tags = ptr->get<std::vector<std::string> >("control_tags");
    if (!tags.empty())
    {
      oss << ConsoleUtils::indent(4) << "Tag(s): ";
      for (std::vector<std::string>::const_iterator it = tags.begin(); it != tags.end(); ++it)
        oss << *it << " ";
      oss << '\n';
    }

    oss << ConsoleUtils::indent(4) << "Parameter(s):\n";
    for (std::set<MooseObjectParameterName>::const_iterator it = iter->second.begin(); it != iter->second.end(); ++it)
      oss << ConsoleUtils::indent(6) << std::setw(ConsoleUtils::console_field_width) << it->parameter() << ptr->type(it->parameter()) << '\n';
  }

  _console << oss.str() << std::endl;

  if (_clear_after_output)
    wh.clearControlledParameters();
}
