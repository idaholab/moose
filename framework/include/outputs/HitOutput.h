//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#pragma once

// libMesh includes
#include "libmesh/parameters.h"

// MOOSE includes
#include "FileOutput.h"
#include "MooseObjectWarehouseBase.h"
#include "MooseApp.h"

// hit
#include "parse.h"

// Outputs complete MOOSE input file that includes all objects, including those added by actions.
//
// I tried several times to create a reliable method to look up for the syntax necessary for
// building an object. The action system allows for multiple tasks associated with an action and
// allows for custom syntax to be injected by applications. This caused the look up to be
// unreliable. Rather than spend a bunch of time trying to come up with logic I feel it is
// better to just use the parameter system to explicitly define how an object should be created.
//
// Therefore, for an object to operate with this output exploder it needs to include a second
// argument to InputParameters::registerBase. If an object doesn't use that and it is contained
// in an input file that is being exploded you get a nice warning about what needs to be done. It
// would be great if it was automatic.
//
// params.registerBase("MeshGenerator", "Mesh/*");
class Hitoutput : public FileOutput
{
public:
  static InputParameters validParams();
  Hitoutput(const InputParameters & parameters);
  virtual void output(const ExecFlagType & type) override;
  virtual std::string filename() override;

protected:
  /**
   * Adds parameter key-value pairs to to the supplied parent object.
   *
   * Parameters are only added if they are valid and not private.
   *
   * @param name The name of the parameter to add.
   * @param value The Value * containing the parameter value.
   * @param parent The parent hit node to contain the parameter.
   * @param parameters The InputParameters object for checking private/valid state.
   */
  void addParameter(const std::string & name,
                    libMesh::Parameters::Value * value,
                    hit::Node * parent,
                    const InputParameters & parameters);

  /**
   * Adds content of InputParameters object to the hit tree.
   *
   * @param root The root node to insert parameters.
   * @param params Pointer to the InputParameter object to add.
   */
  void addInputParameters(hit::Node * root, std::shared_ptr<InputParameters> params);

  /**
   * Helper class to add Action parameters (if needed)
   *
   * Ideally this would not be needed but some actions trigger behavior not captured within a
   * MooseObject
   */
  template <class T>
  void addActionSyntax(const std::string & name,
                       hit::Node * parent,
                       const std::set<std::string> & param_names = {},
                       bool create_sections = false);
};

template <class T>
void
Hitoutput::addActionSyntax(const std::string & name,
                           hit::Node * parent,
                           const std::set<std::string> & param_names,
                           bool create_sections)
{
  // Locate the actions
  std::vector<const T *> actions = _app.actionWarehouse().template getActions<T>();
  // Loop over the actions and add parameters
  if (!actions.empty())
  {
    // Locate/create the node
    hit::Node * node = parent->find(name);
    if (node == nullptr)
    {
      node = new hit::Section(name);
      parent->addChild(node);
    }

    for (const auto action_ptr : actions)
    {
      // The "create_sections" flag create sub-sections for each action (e.g., AddVariableAction)
      hit::Node * section;
      if (create_sections)
      {
        section = new hit::Section(action_ptr->name());
        node->addChild(section);
      }
      else
        section = node;

      for (const auto & map_pair : action_ptr->parameters())
        if (param_names.empty() || param_names.count(map_pair.first))
          addParameter(map_pair.first, map_pair.second, section, action_ptr->parameters());
    }
  }
}
