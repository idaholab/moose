/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "StateSimActTransition.h"
#include "StateSimVariable.h"
#include "StateSimState.h"
#include "StateSimModel.h"
#include <string>
#include <typeinfo>
#include <stdexcept>
#include <algorithm>

StateSimActTransition::StateSimActTransition(StateSimModel & main_model, const std::string &  name)
  : StateSimAction(main_model, name, ACTION_TYPE_ENUM::AT_TRANSITION),
    _mut_excl(false),
    _has_var_probs(false)
{
}

StateSimActTransition::StateSimActTransition(StateSimModel & main_model, const std::string &  name, StateSimState & to_state, const std::string & move_desc)
  : StateSimAction(main_model, name, ACTION_TYPE_ENUM::AT_TRANSITION),
    _mut_excl(false),
    _has_var_probs(false)
{
  this->addToState(to_state, -1, move_desc);
}

void
StateSimActTransition::addToState(StateSimState & to_state, StateSimBase * state_sim_var_prob, const std::string & fail_desc)
{
  if ((state_sim_var_prob == NULL) ||
      (typeid(*state_sim_var_prob) != typeid(StateSimVariable)) ||
      (((StateSimVariable *)state_sim_var_prob)->getVarType() != VAR_TYPE::VT_DOUBLE))
  {
    mooseAssert(false, "state_sim_var_prob parameter must be a Real *");
  }

  _has_var_probs = true;
  StateSimVariable * prob_var = (StateSimVariable *)state_sim_var_prob;

  _to_state_probs.push_back(prob_var->getReal());
  _to_state_var_probs.push_back(prob_var);
  _to_states.push_back(&to_state);
  _fail_desc.push_back(fail_desc);
}

void
StateSimActTransition::addToState(StateSimState & to_state, const Real & prob, const std::string & fail_desc)
{
  if (prob < 0) //use the remainder so put it on the back of the lise.
  {
    _to_state_probs.push_back(prob);
    _to_state_var_probs.push_back(NULL);
    _to_states.push_back(&to_state);
    _fail_desc.push_back(fail_desc);
  }
  else
  {
    _to_state_probs.insert(_to_state_probs.begin(), prob);
    _to_state_var_probs.insert(_to_state_var_probs.begin(), NULL);
    _to_states.insert(_to_states.begin(), &to_state);
    _fail_desc.insert(_fail_desc.begin(), fail_desc);
  }
}

void
StateSimActTransition::removeToState(StateSimState & rem_state)
{
  StateSimState * a = &rem_state;
  //auto func = [&a](const StateSimState * b) -> bool {return a==b;};
  const auto & found_state = std::find_if(_to_states.begin(), _to_states.end(), [&a](const StateSimState * b) { return a == b; });
  if (found_state != _to_states.end())
  {
    int idx = found_state - _to_states.begin();
    _to_states.erase(found_state);
    _to_state_probs.erase(_to_state_probs.begin() + idx);
    _to_state_var_probs.erase(_to_state_var_probs.begin() + idx);
    _fail_desc.erase(_fail_desc.begin() + idx);
  }
}

std::vector<std::shared_ptr<StateSimIdAndDesc>>
StateSimActTransition::WhichToState()
{
  std::vector<std::shared_ptr<StateSimIdAndDesc>> ret_list;

  if (_has_var_probs)
  {
    for (unsigned int i = 0; i < _to_state_var_probs.size(); i++)
    {
      if (_to_state_var_probs[i] != NULL)
      {
        _to_state_probs[i] = _to_state_var_probs[i]->getReal();
      }
    }
  }

  Real prob_sum = 0;
  for (const auto & n : _to_state_probs)
  {
    prob_sum += n;
  }

  if (_mut_excl && (prob_sum > 0) && (prob_sum < 1.0))
    mooseAssert(false, "No default state entered for " + this->name() + " and transitions probabilites don't add up to 1.0.");
  //throw std::runtime_error("No default state entered for " + this->name() + " and transitions probabilites don't add up to 1.0.");
  else if (_to_states.size() == 0) //Not sure if this is an error what if we want to just leave the state.
    mooseAssert(false, "No to states for transition " + this->name());
  //throw std::runtime_error("No to states for transition " + this->name());
  else if ((_to_state_probs[0] == 1.0) || (_to_state_probs[0] == -1.0))
  {
    ret_list.push_back(std::make_shared<StateSimIdAndDesc>(_to_states[0]->id(), _fail_desc[0]));
    return ret_list;
  }

  Real rand_num = nextRandom();
  Real sum = 0.0;
  std::unordered_map<int, int> added;
  for (unsigned int slot = 0; slot < _to_state_probs.size(); slot++)
  {
    int id = _to_states[slot]->id();

    if (_mut_excl) //use the prev bound is the low
    {
      if (rand_num <= _to_state_probs[slot] + sum)
      {
        //ret_list.push_back(std::make_shared<StateSimIdAndDesc>(id, _fail_desc[slot]));
        ret_list.emplace_back(std::make_shared<StateSimIdAndDesc>(id, _fail_desc[slot]));
        //ret_list.emplace_back(id, _fail_desc[slot]);
        return ret_list;
      }

      sum += _to_state_probs[slot];
    }
    else //use the bound (box - prob) as the low
    {
      if (slot == (_to_state_probs.size() - 1))
        break;

      rand_num = nextRandom();
      std::unordered_map<int, int>::const_iterator got = added.find(id);
      if ((rand_num <= _to_state_probs[slot]) && //if not already in the return list add it.
          (got == added.end()))
      {
        ret_list.emplace_back(std::make_shared<StateSimIdAndDesc>(id, _fail_desc[slot]));
        added[id] = id;
      }
    }
  }

  if (!ret_list.empty()) //no probability items were selected we must use the default state
  {
    ret_list.emplace_back(std::make_shared<StateSimIdAndDesc>(_to_states[_to_states.size() - 1]->id(), _fail_desc[_to_states.size() - 1]));
  }

  return ret_list;
}
