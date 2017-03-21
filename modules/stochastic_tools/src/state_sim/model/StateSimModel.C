/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "StateSimModel.h"
#include <string>
#include <cmath>
#include "MooseRandom.h"
#include "StateSimEvFailRate.h"
#include "StateExternalCouplingUO.h"
#include "StateSimBase.h"
#include "StateSimDiagram.h"
#include "StateSimDiagramEval.h"
#include "StateSimState.h"
#include "StateSimActTransition.h"
#include "StateSimActRunCode.h"
#include "StateSimAction.h"
#include "StateSimAllStates.h"
#include "StateSimBitset.h"
#include "StateSimLogicNode.h"
#include "StateSimEvCodeEval.h"
#include "StateSimVariable.h"

bool
evalExtEv1(const TimespanH & cur_ev_time, const TimespanH &, const StateSimBitset &, const StateSimAllVariables & vars)
{
  std::string name = "comp1";
  const StateSimVariable * comp1 = vars[name];
  const StateSimVariable * comp2 = vars["comp2"];
  if (((comp1->getRealPtrVal() > 0.0) && (cur_ev_time == 40.0)) ||
      (comp2->getRealPtrVal() > 0.0))
    return true;
  else
    return false;
}

bool
evalExtEv2(const TimespanH & cur_ev_time, const TimespanH &, const StateSimBitset &, const StateSimAllVariables & vars)
{
  const StateSimVariable * testVar = vars["evVarTest"];
  if (testVar && (testVar->getReal() == cur_ev_time))
    return true;
  else
    return false;
}

void
runCodeTest1(const TimespanH &, const TimespanH &, const StateSimBitset &, StateSimAllVariables & vars)
{
  StateSimVariable * testVar = vars["var1"];
  testVar->setReal(21.0);
}

StateSimModel::StateSimModel(const TimespanH & max_sim_time, const StateExternalCouplingUO & ext_coupling_refs)
  : _time_events(),
    _condition_events(),
    _states(),
    _diagrams(),
    _variables(),
    _actions(),
    _max_sim_time(max_sim_time)
{
  _next_id.fill(0);

  //assign all the external coupling variables with the other variables
  const std::vector<std::string> & vpp_lookup_name = ext_coupling_refs.getStateExtCouplingNames();
  for (unsigned long i = 0; i < vpp_lookup_name.size(); ++i)
  {
    _variables.add(new StateSimVariable(*this, vpp_lookup_name[i], ext_coupling_refs.getStateExtCouplingValue(i)));
    //_variables.add(std::make_shared<StateSimVariable>(vpp_lookup_name[i], ext_coupling_refs.getStateExtCouplingValue(i)));
  }

  //add test items
  test();

  for (unsigned long i = 0; i < vpp_lookup_name.size(); ++i)
  {
    _condition_events.add(new StateSimEvCodeEval(*this, vpp_lookup_name[i], &evalExtEv1));
  }
}

int
StateSimModel::nextID(STATESIM_TYPE obj_type)
{
  auto idx = static_cast<int>(obj_type);
  mooseAssert(idx < (int)_next_id.size(), "Error obj ID index larger than array size. - " + std::to_string(idx));
  return _next_id[idx]++;
}

bool
StateSimModel::test()
{
  StateSimDiagram t_diag(*this, "Main", DIAGRAM_TYPE_ENUM::DT_PLANT);
  StateSimDiagramEval t_diag2(*this, "Main", DIAGRAM_TYPE_ENUM::DT_COMPONENT);

  StateSimAllStates state_list;

  StateSimState * t_state = new StateSimState(*this, "state", STATE_TYPE_ENUM::ST_STANDARD, t_diag);
  state_list.add(t_state);

  StateSimState * t_state2(new StateSimState(*this, "state2", STATE_TYPE_ENUM::ST_STANDARD, t_diag));
  state_list.add(t_state2);

  StateSimState * check_state = state_list["state"];

  t_state->setDesc("Hello");
  if (check_state->desc() != "Hello")
    return false;

  StateSimEvFailRate t_ev(*this, "ev", 2.3, 365, 365);//StateSimBase::YEAR_TIME, StateSimBase::YEAR_TIME);

  StateSimActTransition t_act_move(*this, "act_move", *t_state2, "test move");
  //t_state->addEvent(t_ev, FALSE_REF, &t_act_move);
  t_state->addEvent(t_ev, false, &t_act_move);
  t_state->addActionForEvent(t_ev, t_act_move, NULL);

  t_ev.setDesc("changed desc");
  const StateSimEvent & ret_ev = t_state->getEvent(0);
  if (ret_ev.desc() != "changed desc")
    return false;

  StateSimLogicNode top(*this, "top", GATE_TYPE_ENUM::GT_OR, NULL);

  StateSimLogicNode g1(*this, "g1", GATE_TYPE_ENUM::GT_AND, &top);
  g1.addCompChild(t_diag2);

  top.addGateChild(g1);

  StateSimBitset cur_states(10);
  t_diag2.addState(*t_state, State_Val::TRUE_VAL);
  if (t_diag2.evaluate(cur_states, true) == State_Val::TRUE_VAL)
    return false;

  if (top.evaluate(cur_states, true))
    return false;

  cur_states.set(t_state->id(), true); //now things should evaluate to true

  if (t_diag2.evaluate(cur_states, true) != State_Val::TRUE_VAL)
    return false;

  if (top.evaluate(cur_states, true) == false)
    return false;

  //StateSimVariable & x = _variables.add(new StateSimVariable("var1", VS_GLOBAL, 20.0));
  _variables.add(new StateSimVariable(*this, "var1", VAR_SCOPE_ENUM::VS_GLOBAL, 20.0));
  //StateSimEvCodeEval * codeEv = new StateSimEvCodeEval("evVarTest", &evalExtEv2);
  //condition_events.add(codeEv);
  //StateSimEvCodeEval & codeEv = dynamic_cast<StateSimEvCodeEval &>(condition_events.add(new StateSimEvCodeEval("evVarTest", &evalExtEv2)));
  _condition_events.add(new StateSimEvCodeEval(*this, "evVarTest", &evalExtEv2));

  //StateSimActRunCode * a = new StateSimActRunCode("change_var1", &runCodeTest1);
  //StateSimActRunCode & b = dynamic_cast<StateSimActRunCode&>(actions.add(a));
  //b.runCode(0.0, 0.0, cur_states, *this);

  StateSimActRunCode & codeAct = dynamic_cast<StateSimActRunCode &>(_actions.add(new StateSimActRunCode(*this, "change_var1", &runCodeTest1)));
  //StateSimActRunCode & codeAct2 = (StateSimActRunCode)codeAct;
  codeAct.runCode(0.0, 0.0, cur_states, *this);

  _time_events.add(new StateSimEvFailRate(*this, "A", 0.1, TimespanH(1), TimespanH(24)));
  _time_events.add(new StateSimEvFailRate(*this, "B", 0.1, TimespanH(1), TimespanH(24)));
  _time_events.add(new StateSimEvFailRate(*this, "C", 0.1, TimespanH(1), TimespanH(24)));
  _time_events.add(new StateSimEvFailRate(*this, "D", 0.1, TimespanH(1), TimespanH(24)));
  _time_events.add(new StateSimEvFailRate(*this, "E", 0.1, TimespanH(1), TimespanH(24)));
  _time_events.add(new StateSimEvFailRate(*this, "F", 0.1, TimespanH(1), TimespanH(24)));

  //t_state1.add

  return true;
}
