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

#ifndef ACTIONWAREHOUSE_H
#define ACTIONWAREHOUSE_H

#include <string>
#include <set>
#include <map>
#include <ostream>

#include "Action.h"
#include "libmesh/exodusII_io.h"

/// Typedef to hide implementation details
typedef std::vector<Action *>::iterator ActionIterator;

class MooseMesh;
class Executioner;
class Syntax;
class ActionFactory;

/**
 * Storage for action instances.
 */
class ActionWarehouse
{
public:
  ActionWarehouse(MooseApp & app, Syntax & syntax, ActionFactory & factory);
  ~ActionWarehouse();

  void build();
  void clear();

  bool empty() { return _action_blocks.empty(); }

  void addActionBlock(Action * blk);

  /**
   * This method checks the actions stored in the warehouse against the list of required registered
   * actions to see if all of them have been satisfied.  It should be called before running
   * a MOOSE problem
   */
  void checkUnsatisfiedActions() const;

  void printActionDependencySets();
  void printInputFile(std::ostream & out);

  ActionIterator actionBlocksWithActionBegin(const std::string & action_name);
  ActionIterator actionBlocksWithActionEnd(const std::string & action_name);

  const std::vector<Action *> & getActionsByName(const std::string & action_name) const;

  /**
   * This method loops over all actions in the warehouse and executes them.  Meta-actions
   * may add new actions to the warehouse on the fly and they will still be executed in order
   */
  void executeAllActions();

  /**
   * This method executes only the actions in the warehouse that satisfy the action_name
   * passed in
   */
  void executeActionsWithAction(const std::string & name);

  void showActions(bool state = true) { _show_actions = state; }

  //// Getters
  Syntax & syntax() { return _syntax; }

  MooseMesh * & mesh() { return _mesh; }
  MooseMesh * & displacedMesh() { return _displaced_mesh; }
  FEProblem * & problem() { return _problem; }
  ExodusII_IO * & exReader() { return _exreader; }
  Executioner * & executioner() { return _executioner; }
  MooseApp & mooseApp() { return _app; }

protected:
  void buildBuildableActions(const std::string &action_name);

  /// The MooseApp this Warehouse is associated with
  MooseApp & _app;
  /// Reference to a "syntax" of actions
  Syntax & _syntax;
  /// The Factory that builds Actions
  ActionFactory & _action_factory;
  /// Pointers to the actual parsed input file blocks
  std::map<std::string, std::vector<Action *> > _action_blocks;
  /// The container that holds the sorted action names from the DependencyResolver
  std::vector<std::string> _ordered_names;
  /// Use to store the current list of unsatisfied dependencies
  std::set<std::string> _unsatisfied_dependencies;

  /**
   *  Flag to indicate whether or not there is an active iterator on this class.
   *  There can only be a single active iterator because of the potential for
   *  meta Actions to add new Actions into the warehouse on the fly
   */
  bool _generator_valid;

  // DEBUGGING
  bool _show_actions;


  //
  // data created by actions
  //

  /// Mesh class
  MooseMesh * _mesh;
  /// Possible mesh for displaced problem
  MooseMesh * _displaced_mesh;
  /// Problem class
  FEProblem * _problem;
  /// Auxiliary object for restart
  ExodusII_IO * _exreader;
  /// Executioner for the simulation (top-level class, is stored in MooseApp, where it is freed)
  Executioner * _executioner;

};

#endif // ACTIONWAREHOUSE_H
