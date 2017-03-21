/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef STATESIMACTIONLIST_H
#define STATESIMACTIONLIST_H

#include "StateSimBase.h"
#include <string>
#include <unordered_map>

//Forward Declarations
class StateSimAction;

class StateSimActionList
{
public:
  /**
   * This is the main constructor for the Action list.
   * @param move_from_current - item moves from the parent state if an action is executed
   */
  StateSimActionList(bool move_from_current = false);
  void add(StateSimAction * to_add);
  void clear();
  /**
   * addRange - Add the given list of items to this action list.
   * @param StateSimActionList - list of item to add to this list
   */
  void addRange(StateSimActionList * to_add);
  bool contains(const StateSimAction to_find);
  int size() {return _actions.size();};
  /**
   * moveFromCurrent - move from the parent state if any of these actions are executed.
   * @return  flag indicating if moving
   */
  bool moveFromCurrent() {return _move_from_current;};
  /**
   * moveFromCurrent - move from the parent state if any of these actions are executed.
   * @param move - new flag value indicating if moving
   */
  void moveFromCurrent(const bool & move) {_move_from_current = move;};

  //todo
  //getJSON
  //deserializeJSON

protected:
  bool _move_from_current;
  std::unordered_map<int, StateSimAction *> _actions;

private:
  //
};

#endif
