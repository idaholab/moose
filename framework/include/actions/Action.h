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

#ifndef ACTION_H
#define ACTION_H

#include "InputParameters.h"

#include <string>
#include <ostream>

class Action;
class ActionWarehouse;
class ActionFactory;
class MooseMesh;
class FEProblem;
class Executioner;
class MooseApp;
class Factory;

template<>
InputParameters validParams<Action>();

/**
 * Base class for actions.
 */
class Action
{
public:
  Action(const std::string & name, InputParameters params);
  virtual ~Action() {}                  // empty virtual destructor for proper memory release

  virtual void act() = 0;

  const std::string & name() const { return _name; }

  const std::string & type() const { return _action_type; }

  InputParameters & parameters() { return _pars; }

  const std::string & specificTaskName() const { return _specific_task_name; }

  const std::set<std::string> & getAllTasks() const { return _all_tasks; }

  template <typename T>
  const T & getParam(const std::string & name) { return _pars.get<T>(name); }

  template <typename T>
  const T & getParam(const std::string & name) const { return _pars.get<T>(name); }

  inline bool isParamValid(const std::string &name) const { return _pars.isParamValid(name); }

  inline InputParameters & getParams() { return _pars; }

  /**
   * Returns the short name which is the final string after the last delimiter for the
   * current ParserBlock
   */
  std::string getShortName() const;

  void appendTask(const std::string & task) { _all_tasks.insert(task); }

protected:
  /// The name of the action
  std::string _name;

  /// Input parameters for the action
  InputParameters _pars;

  // The registered syntax for this block if any
  std::string _registered_identifier;

  // The type name of this Action instance
  std::string _action_type;

  /// The MOOSE application this is associated with
  MooseApp & _app;

  /// The Factory associated with the MooseApp
  Factory & _factory;

  /// Builds Actions
  ActionFactory & _action_factory;

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

  /// The current action (even though we have seperate instances for each action)
  const std::string & _current_action;

  MooseMesh * & _mesh;
  MooseMesh * & _displaced_mesh;
  /// Convenience reference to a problem this action works on
  FEProblem * & _problem;
  /// Convenience reference to an executioner
  Executioner * & _executioner;
};

#endif // ACTION_H
