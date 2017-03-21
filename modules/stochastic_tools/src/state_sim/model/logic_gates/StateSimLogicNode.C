/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "StateSimLogicNode.h"
#include "StateSimBitset.h"
#include "StateSimModel.h"
#include <string>
//#include "NextObjID.h"

StateSimLogicNode::StateSimLogicNode(StateSimModel & main_model, const std::string &  name)
  : StateSimBase(main_model.nextID(STATESIM_TYPE::LOGIC_GATE), name),
    _gate_type(GATE_TYPE_ENUM::GT_OR),
    _root_parent(NULL),
    _n(0)
{
}

StateSimLogicNode::StateSimLogicNode(StateSimModel & main_model, const std::string &  name, GATE_TYPE_ENUM gate_type, StateSimLogicNode * root_parent, const int & n_val)
  : StateSimBase(main_model.nextID(STATESIM_TYPE::LOGIC_GATE), name),
    _gate_type(gate_type),
    _root_parent(root_parent),
    _comp_children(),
    _sub_gates(),
    _n(n_val)
{
}

bool
StateSimLogicNode::evaluate(const StateSimBitset & cur_states, const bool & success_space) const
{
  bool ret_val = false;
  int cnt = 0;

  switch (_gate_type)
  {
    case GATE_TYPE_ENUM::GT_AND:
    case GATE_TYPE_ENUM::GT_NOT:
      ret_val = true;
      break;

    case GATE_TYPE_ENUM::GT_OR:
    case GATE_TYPE_ENUM::GT_NOFM:
      ret_val = false;
      break;

    default:
      ret_val = false;
      break;
  }

  for (auto cur_comp : _comp_children)
  {
    switch (_gate_type)
    {
      case GATE_TYPE_ENUM::GT_AND:
        ret_val = (ret_val && (cur_comp.second->evaluate(cur_states, success_space) == State_Val::TRUE_VAL));
        if (!ret_val)
        {
          return ret_val;
        }
        break;

      case GATE_TYPE_ENUM::GT_OR:
        ret_val = (ret_val || (cur_comp.second->evaluate(cur_states, success_space) == State_Val::TRUE_VAL));
        if (ret_val)
        {
          return ret_val;
        }
        break;

      case GATE_TYPE_ENUM::GT_NOT:
        return (cur_comp.second->evaluate(cur_states, success_space) != State_Val::TRUE_VAL);

      case GATE_TYPE_ENUM::GT_NOFM:
      {
        bool imputVal = (cur_comp.second->evaluate(cur_states, true)  == State_Val::TRUE_VAL);
        if ((imputVal && success_space) || (!imputVal && !success_space))
        {
          ++cnt;
          if (cnt >= _n)
          {
            return true;
          }
        }
        break;
      }
    }
  }

  for (auto cur_node : _sub_gates)
  {
    switch (_gate_type)
    {
      case GATE_TYPE_ENUM::GT_AND:
        ret_val = (ret_val && cur_node.second->evaluate(cur_states, success_space));
        if (!ret_val)
        {
          return ret_val;
        }
        break;

      case GATE_TYPE_ENUM::GT_OR:
        ret_val = (ret_val || cur_node.second->evaluate(cur_states, success_space));
        if (ret_val)
        {
          return ret_val;
        }
        break;

      case GATE_TYPE_ENUM::GT_NOT:
        return (cur_node.second->evaluate(cur_states, success_space));

      case GATE_TYPE_ENUM::GT_NOFM:
        if (cur_node.second->evaluate(cur_states, success_space))
        {
          ++cnt;
          if (cnt >= _n)
          {
            return true;
          }
        }
        break;
    }
  }

  return ret_val; //todo
}

std::vector<int>
StateSimLogicNode::allUsedStateIDs() const
{
  std::vector<int> ret_ids;

  //for (it_type iterator = _comp_children.begin(); iterator != _comp_children.end(); iterator++)
  for (const auto & pair : _comp_children)
  {
    auto cur_child_state_ids = pair.second->stateIDs();
    ret_ids.insert(ret_ids.end(), cur_child_state_ids.begin(), cur_child_state_ids.end());
  }

  for (const auto & pair : _sub_gates)
  {
    const auto & sub_gate_state_ids = pair.second->allUsedStateIDs();
    ret_ids.insert(ret_ids.end(), sub_gate_state_ids.begin(), sub_gate_state_ids.end());
  }

  sort(ret_ids.begin(), ret_ids.end());
  ret_ids.erase(unique(ret_ids.begin(), ret_ids.end()), ret_ids.end());
  return ret_ids;
}

std::vector<StateSimState *>
StateSimLogicNode::allUsedStates() const
{
  std::vector<StateSimState *> ret_states;

  //for (it_type iterator = _comp_children.begin(); iterator != _comp_children.end(); iterator++)
  for (const auto & pair : _comp_children)
  {
    const auto & cur_child_state_ids = pair.second->states();
    ret_states.insert(ret_states.end(), cur_child_state_ids.begin(), cur_child_state_ids.end());
  }

  for (const auto & i : _sub_gates)
  {
    const auto & sub_gate_state_ids = i.second->allUsedStates();
    ret_states.insert(ret_states.end(), sub_gate_state_ids.begin(), sub_gate_state_ids.end());
  }

  sort(ret_states.begin(), ret_states.end());
  ret_states.erase(unique(ret_states.begin(), ret_states.end()), ret_states.end());
  return ret_states;
}
