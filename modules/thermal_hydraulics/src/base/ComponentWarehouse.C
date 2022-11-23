//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComponentWarehouse.h"
#include "Component.h"
#include "DependencyResolver.h"
#include "Factory.h"

ComponentWarehouse::ComponentWarehouse(Factory & factory) : _factory(factory) {}

bool
ComponentWarehouse::hasComponent(const std::string & name) const
{
  auto it = _comp_by_name.find(name);
  return (it != _comp_by_name.end());
}

void
ComponentWarehouse::addComponent(const std::string & type,
                                 const std::string & name,
                                 InputParameters & params)
{
  if (hasComponent(name))
    mooseError("Component with name '", name, "' already exists");
  else
  {
    std::shared_ptr<Component> comp = _factory.create<Component>(type, name, params);
    _comp_by_name[name] = comp;
    _components.push_back(comp);
  }
}

void
ComponentWarehouse::sortComponents()
{
  DependencyResolver<std::shared_ptr<Component>> dependency_resolver;
  for (const auto & component : _components)
  {
    dependency_resolver.addNode(component);
    for (const auto & dependency : component->getDependencies())
      if (hasComponent(dependency))
        dependency_resolver.addEdge(_comp_by_name[dependency], component);
  }

  _components = dependency_resolver.dfs();
}
