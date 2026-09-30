//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ClosuresRegistry.h"
#include "ActionWarehouse.h"
#include "ActionFactory.h"
#include "ClosuresBase.h"

registerMooseAction("ThermalHydraulicsApp", ClosuresRegistry, "THM:add_closures");

InputParameters
ClosuresRegistry::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription(
      "Problem-agnostic, name-keyed registry of Closures objects (see ClosuresBase). "
      "Auto-created; not meant to be declared in an input file.");
  return params;
}

ClosuresRegistry::ClosuresRegistry(const InputParameters & params) : Action(params) {}

void
ClosuresRegistry::addClosures(const std::string & type,
                              const std::string & name,
                              InputParameters params)
{
  if (_closures_by_name.find(name) != _closures_by_name.end())
    mooseError("A closures object with the name '", name, "' already exists.");

  _closures_by_name[name] = _factory.create<ClosuresBase>(type, name, params);
}

bool
ClosuresRegistry::hasClosures(const std::string & name) const
{
  return _closures_by_name.find(name) != _closures_by_name.end();
}

std::shared_ptr<ClosuresBase>
ClosuresRegistry::getClosures(const std::string & name) const
{
  const auto it = _closures_by_name.find(name);
  if (it == _closures_by_name.end())
    mooseError("The requested closures object '", name, "' does not exist.");
  return it->second;
}

void
ClosuresRegistry::emitLoggedMessages() const
{
  _log.emitLoggedWarnings();
  _log.emitLoggedErrors();
}

ClosuresRegistry &
ClosuresRegistry::findOrCreate(ActionWarehouse & awh, ActionFactory & action_factory)
{
  const auto existing = awh.getActions<ClosuresRegistry>();
  if (!existing.empty())
    return *const_cast<ClosuresRegistry *>(existing[0]);

  const std::string class_name = "ClosuresRegistry";
  InputParameters params = action_factory.getValidParams(class_name);
  params.set<bool>("_built_by_moose") = true;
  params.set<std::string>("registered_identifier") = "(AutoBuilt)";

  auto action = action_factory.create(class_name, "closures_registry", params);
  auto * registry = dynamic_cast<ClosuresRegistry *>(action.get());
  if (!registry)
    ::mooseError("Internal error: failed to create the ClosuresRegistry");
  awh.addActionBlock(action);
  return *registry;
}
