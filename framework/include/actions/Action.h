//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ParallelParamObject.h"
#include "InputParameters.h"
#include "MeshMetaDataInterface.h"
#include "Registry.h"
#include "PerfGraphInterface.h"
#include "MooseObjectParameterName.h"

#include <string>
#include <ostream>

class ActionWarehouse;
class ActionFactory;
class MooseMesh;
class FEProblemBase;
class Executioner;
class MooseApp;
class Factory;

/**
 * Base class for actions.
 */
class Action : public ParallelParamObject, public MeshMetaDataInterface, public PerfGraphInterface
{
public:
  static InputParameters validParams();

  Action(const InputParameters & parameters);

  virtual ~Action() = default;

  /**
   * The method called externally that causes the action to act()
   */
  void timedAct();

private:
  /**
   * Method for adding a single relationship manager
   * @param input_rm_type What relationship manager type we are currently adding
   * @param moose_object_pars The parameters of the MooseObject that requested the RM
   * @param rm_name The class type of the RM, e.g. ElementSideNeighborLayers
   * @param rm_type The RelationshipManagerType, e.g. geometric, algebraic, coupling
   * @param rm_input_parameter_func The RM callback function, typically a lambda defined in the
   *                                requesting MooseObject's validParams function
   * @param sys_type A RMSystemType that can be used to limit the systems and consequent dof_maps
   *                 that the RM can be attached to
   * @return Whether a relationship manager was added
   */
  bool
  addRelationshipManager(Moose::RelationshipManagerType input_rm_type,
                         const InputParameters & moose_object_pars,
                         std::string rm_name,
                         Moose::RelationshipManagerType rm_type,
                         Moose::RelationshipManagerInputParameterCallback rm_input_parameter_func,
                         Moose::RMSystemType sys_type = Moose::RMSystemType::NONE);

protected:
  /**
   * Method to add a relationship manager for the objects being added to the system. Relationship
   * managers have to be added relatively early. In many cases before the Action::act() method
   * is called.
   * @param when_type The parameter indicating the normal time for adding either Geometric or
   *        Algebraic RelationshipManagers. It may not always be possible to add your
   *        RelationshipManager as early as you'd like. In these cases, your DistributedMesh may
   *        consume more memory during the problem setup.
   * @param moose_object_pars The MooseObject to inspect for RelationshipManagers to add
   * @return Whether a relationship manager was added
   */
  bool addRelationshipManagers(Moose::RelationshipManagerType when_type,
                               const InputParameters & moose_object_pars);

public:
  /**
   * Method to add a relationship manager for the objects being added to the system. Relationship
   * managers have to be added relatively early. In many cases before the Action::act() method
   * is called.
   * @param when_type The parameter indicating the normal time for adding either Geometric or
   *        Algebraic RelationshipManagers. It may not always be possible to add your
   *        RelationshipManager as early as you'd like. In these cases, your DistributedMesh may
   *        consume more memory during the problem setup.
   */
  virtual void addRelationshipManagers(Moose::RelationshipManagerType when_type);

  /**
   * The unique name for accessing input parameters of this action in the InputParameterWarehouse
   */
  MooseObjectName uniqueActionName() const { return uniqueName(); }

  const std::string & specificTaskName() const { return _specific_task_name; }

  const std::set<std::string> & getAllTasks() const { return _all_tasks; }

  void appendTask(const std::string & task) { _all_tasks.insert(task); }

protected:
  /**
   * Method to add objects to the simulation or perform other setup tasks.
   */
  virtual void act() = 0;

  /**
   * Associates the object's parameters \p params with the input location from this
   * Action's parameter with the name \p param_name, if one exists.
   *
   * For example, you have a parameter in this action of type bool with name "add_mesh".
   * You then add an action within this action that creates a mesh if this param is
   * true. If you call associateWithParameter("add_mesh", action_params) where
   * action_params are the parameters for the action, we then associate that action
   * with the "add_mesh" parameter. Therefore, the resulting created mesh will also
   * be associated with the "add_mesh" param and any errors that are non-parameter errors
   * (i.e., mooseError/mooseWarning) will have the line context of the "add_mesh"
   * parameter in this action. The same goes for any errors that are produce within
   * the created action.
   */
  void associateWithParameter(const std::string & param_name, InputParameters & params) const;

  /**
   * The same as associateWithParameter() without \p from_params, but instead
   * allows you to associate this with another object's parameters instead of the
   * parameters from this action.
   *
   * An example here is when you want to associate the creation of an action with
   * an argument from the application.
   */
  void associateWithParameter(const InputParameters & from_params,
                              const std::string & param_name,
                              InputParameters & params) const;

  // The registered syntax for this block if any
  std::string _registered_identifier;

  /**
   * This member will only be populated if this Action instance is only designed to
   * handle one task.  This happens when an Action is registered with several pieces
   * of syntax in which case separate instances are built to handle the different
   * incoming parameter values.
   */
  std::string _specific_task_name;

  /**
   * A list of all the tasks that this Action will satisfy.
   * Note: That this is _not_ populated at construction time.  However, all tasks will be
   *       added prior to act().
   */
  std::set<std::string> _all_tasks;

  /// Reference to ActionWarehouse where we store object build by actions
  ActionWarehouse & _awh;

  /// The current action (even though we have separate instances for each action)
  const std::string & _current_task;

  std::shared_ptr<MooseMesh> & _mesh;
  std::shared_ptr<MooseMesh> & _displaced_mesh;

  /// Convenience reference to a problem this action works on
  std::shared_ptr<FEProblemBase> & _problem;

  /// Timers
  PerfID _act_timer;

  // Base classes have the same name for that attribute, pick one
  using MooseBase::_app;
};
