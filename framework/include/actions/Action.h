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

  const std::string & name() { return _name; }

  InputParameters & parameters() { return _pars; }

  const std::string & getTask() { return _current_action; }

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

protected:
  /// The name of the action
  std::string _name;
  /// Input parameters for the action
  InputParameters _pars;

  /// The MOOSE application this is associated with
  MooseApp & _app;

  /// The Factory associated with the MooseApp
  Factory & _factory;

  /// Builds Actions
  ActionFactory & _action_factory;

  /// The current action (even though we have seperate instances for each action)
  std::string _current_action;

  /// Reference to ActionWarehouse where we store object build by actions
  ActionWarehouse & _awh;
  MooseMesh * & _mesh;
  MooseMesh * & _displaced_mesh;
  /// Convenience reference to a problem this action works on
  FEProblem * & _problem;
  /// Convenience reference to an executioner
  Executioner * & _executioner;
};

#endif // ACTION_H
