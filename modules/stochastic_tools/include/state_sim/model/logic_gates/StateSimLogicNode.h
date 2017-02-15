/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef STATESIMLOGICNODE_H
#define STATESIMLOGICNODE_H

#include "StateSimBase.h"
#include "StateSimDiagramEval.h" //cant user forward declaration
#include <string>
#include <vector>
#include <unordered_map>

//Forward Declarations
class StateSimBitset;
class StateSimState;

//Forward Declarations
class StateSimModel;
//class StateSimDiagramEval;

/**
 * A Logic Node is a boolean gate that evaluates the assiged child gates or Eval Diagram.
 */
class StateSimLogicNode : public StateSimBase
{
public:
  /**
   * Not for the User, use .
   * @param name - name of the item
   * @param gate_type - type of the gate.
   * @param root_parent - the top gate for this logic tree
   * @param m_value - n value for an n-or-m gate
   */
  StateSimLogicNode(StateSimModel & main_model, const std::string & name);

  /**
   * This is the main construter for the user.
   * @param name - name of the item
   * @param gate_type - type of the gate.
   * @param root_parent - the top gate for this logic tree
   * @param m_value - n value for an n-or-m gate
   */
  StateSimLogicNode(StateSimModel & main_model, const std::string & name, GATE_TYPE_ENUM gate_type, StateSimLogicNode * root_parent, const int & n_val = 0);

  /**
   * evaluate - caclulate the value of this node
   * @param cur_states - states this model is currently in.
   * @param success_space - return true if the logic evalates to 1.
   * TODO ? change to State_Val if UNKNOWN is needed
   */
  bool evaluate(const StateSimBitset & cur_states, const bool & success_space) const;

  //todo
  //void AddRelatedItems(int item_id);
  //getDerivedJSON
  //getJSON
  //GetTreeJSON
  //LoadObjLinks

  /**
   * addGateChild - add a child gate to this node
   * @param child - child to add
   */
  void addGateChild(StateSimLogicNode & child)
  {
    _sub_gates[child.id()] = &child;
  };

  /**
   * removeGateChild - remove a child gate in this node
   * @param child - child to remove
   */
  void removeGateChild(const StateSimLogicNode & child)
  {
    _sub_gates.erase(child.id());
  };

  /**
   * addCompChild - add a child component to this node
   * @param child - child to add
   */
  void addCompChild(StateSimDiagramEval & child)
  {
    _comp_children[child.id()] = &child;
  };

  /**
   * removeCompChild - remove a child component in this node
   * @param child - child to remove
   */
  void removeCompChild(const StateSimDiagramEval & child)
  {
    _comp_children.erase(child.id());
  };

  /**
   * allUsedStateIDS - get all the state IDs used under this logic tree
   * @return IDs used under this logic tree
   */
  std::vector<int> allUsedStateIDs() const;

  /**
   * allUsedStates - get all the states used under this logic tree
   * @return States used under this logic tree
   */
  std::vector<StateSimState *> allUsedStates() const;

protected:
  GATE_TYPE_ENUM _gate_type;
  StateSimLogicNode * _root_parent;
  std::unordered_map<int, StateSimDiagramEval *> _comp_children;
  std::unordered_map<int, StateSimLogicNode *> _sub_gates;
  int _n;
};

#endif
