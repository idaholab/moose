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

#include "SetupPreconditionerAction.h"
#include "ActionFactory.h"

// Static member initialization
unsigned int SetupPreconditionerAction::_count = 0;
std::map<std::string, std::string> SetupPreconditionerAction::_type_to_action;

template<>
InputParameters validParams<SetupPreconditionerAction>()
{
  InputParameters params = validParams<MooseObjectAction>();
  /**
   * Preconditioners are not normal "Moose Objects" but we do want to build the right Actions
   * and have parameters filled in at construction like normal Moose Object Action so we will
   * use the MooseObjectAction class so the additional parameters from the corresponding Action
   * classes are built for us
   */
  params.addPrivateParam("skip_param_construction", true);
  params.addRequiredParam<std::string>("type", "The Preconditioning type: [PBP, FDP, SMP]");
  return params;
}

SetupPreconditionerAction::SetupPreconditionerAction(const std::string & name, InputParameters params) :
  MooseObjectAction(name, params)
{
  // Init static map
  if (_type_to_action.size() == 0)
  {
    _type_to_action["FDP"] = "SetupFiniteDifferencePreconditionerAction";
    _type_to_action["PBP"] = "SetupPBPAction";
    _type_to_action["SMP"] = "SetupSMPAction";
  }

  /**
   * This is actually an instance of THIS class an not a derived class so we need to construct
   * a parameters object that will be filled in and used by the derived class object.  Instead
   * of using RTTI to determine what type of object this is - we can simply inspect the
   * "action_name" instead
   */
  if (getParam<std::string>("action") != "add_preconditioning")
    _moose_object_pars = ActionFactory::instance()->getValidParams(_type_to_action[_type]);
}

void
SetupPreconditionerAction::act()
{
  Action *action;
  
  // We are only allowed to have one instance of a preconditioner active at a time
  if (++_count > 1)
    mooseError(std::string("More than one active Preconditioner Action detected while building _") + _type + "_");

  // A bit of hackery here to get the right parameters into the new params object
  _moose_object_pars += _pars;
  _moose_object_pars.set<std::string>("action") = "add_preconditioning";
  
  // Add Preconditioning Action to the warehouse on the fly
  action = ActionFactory::instance()->create(_type_to_action[_type], _moose_object_pars);
  Moose::action_warehouse.addActionBlock(action);
}
