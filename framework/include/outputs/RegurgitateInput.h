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

#ifndef REGURGITATEINPUT_H
#define REGURGITATEINPUT_H

// libMesh includes
#include "libmesh/parameters.h"

// MOOSE includes
#include "FileOutput.h"
#include "MooseObjectWarehouseBase.h"
#include "MooseApp.h"

// hit
#include "parse.h"

class RegurgitateInput;

template <>
InputParameters validParams<RegurgitateInput>();

/**
 * Outputs complete MOOSE input file that includes all objects, including those added by actions.
 */
class RegurgitateInput : public FileOutput
{
public:
  RegurgitateInput(const InputParameters & parameters);
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

  template <class T>
  void addActionSyntax(const std::string & name, hit::Node * parent, bool create_sections = false);

  /// Storage for _moose_base relation to the syntax needed to recreated.
  std::map<std::string, std::pair<std::string, bool>> _base_to_syntax;
};

template <class T>
void
RegurgitateInput::addActionSyntax(const std::string & name,
                                  hit::Node * parent,
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

      // Loop over parameters and create a hit::Field for each
      for (const auto & map_pair : action_ptr->parameters())
        addParameter(map_pair.first, map_pair.second, section, action_ptr->parameters());
    }
  }
}

#endif
