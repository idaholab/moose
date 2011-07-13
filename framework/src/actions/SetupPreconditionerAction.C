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
#include "PetscSupport.h"

// Static member initialization
unsigned int SetupPreconditionerAction::_count = 0;
std::map<std::string, std::string> SetupPreconditionerAction::_type_to_action;

template<>
InputParameters validParams<SetupPreconditionerAction>()
{
  InputParameters params = validParams<MooseObjectAction>();
  
  /**
   * This parameter is here to instruct the parent class not to look in the factory for
   * the right MooseObject to build.  It will not exist and fail so we will ask it to just
   * skip it for now so we can handle it in our own constructor
   */
  params.addPrivateParam("skip_param_construction", true);
  params.addRequiredParam<std::string>("type", "The Preconditioning type: [PBP, FDP, SMP]");

#ifdef LIBMESH_HAVE_PETSC
  params.addParam<std::vector<std::string> >("petsc_options", "Singleton Petsc options");
  params.addParam<std::vector<std::string> >("petsc_options_iname", "Names of Petsc name/value pairs");
  params.addParam<std::vector<std::string> >("petsc_options_value", "Values of Petsc name/value pairs (must correspond with \"petsc_options_iname\"");
#endif //LIBMESH_HAVE_PETSC

  return params;
}

SetupPreconditionerAction::SetupPreconditionerAction(const std::string & name, InputParameters params) :
  MooseObjectAction(name, params),
  _is_base_instance(getParam<std::string>("action") != "add_preconditioning" ? true : false)
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
   * a parameters object that will be filled in and used by the derived class object.
   */
  if (_is_base_instance)
    _moose_object_pars = ActionFactory::instance()->getValidParams(_type_to_action[_type]);
}

void
SetupPreconditionerAction::act()
{
  /**
   * We only want to run this "act" on base class instances on the off chance that a derived
   * class doesn't override "act".  We'd fall into an infinite loop otherwise
   */
  if (_is_base_instance)
  {
    Action *action;
    
    // We are only allowed to have one instance of a preconditioner active at a time
    // TODO: We may need to reset this parameter to zero for loosely coupled systems
    if (++_count > 1)
      mooseError(std::string("More than one active Preconditioner Action detected while building _") + _type + "_");

    // A bit of hackery here to get the right parameters into the new params object
    _moose_object_pars += _pars;
    _moose_object_pars.set<std::string>("action") = "add_preconditioning";
  
    // Add Preconditioning Action to the warehouse on the fly
    action = ActionFactory::instance()->create(_type_to_action[_type], _moose_object_pars);
    Moose::action_warehouse.addActionBlock(action);

    /**
     * Go ahead and set common precondition options here.  The child classes will still be called
     * through the action warehouse
     */

#ifdef LIBMESH_HAVE_PETSC
    std::vector<std::string> petsc_options,  petsc_inames, petsc_values;
    petsc_options = getParam<std::vector<std::string> >("petsc_options");
    petsc_inames = getParam<std::vector<std::string> >("petsc_options_iname");
    petsc_values = getParam<std::vector<std::string> >("petsc_options_value");
    
    Moose::PetscSupport::petscSetOptions(petsc_options, petsc_inames, petsc_values);
#endif //LIBMESH_HAVE_PETSC
  }
}

std::string
SetupPreconditionerAction::getTypeString(const std::string & action)
{ 
  for (std::map<std::string, std::string>::const_iterator iter = _type_to_action.begin();
       iter != _type_to_action.end();
       ++iter)
    if (iter->second == action)
      return iter->first;

  return std::string();
}
