/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

#include "SubChannelCreateProblemAction.h"
#include "ActionWarehouse.h"
#include "ActionFactory.h"
#include "AddVariableAction.h"
#include "SubChannelApp.h"
#include "FEProblemBase.h"

registerMooseAction("SubChannelApp", SubChannelCreateProblemAction, "create_problem");

InputParameters
SubChannelCreateProblemAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addRequiredParam<std::string>("type", "Problem type");
  params.addParam<std::string>("name", "SubChannel Problem", "The name the problem");
  return params;
}

SubChannelCreateProblemAction::SubChannelCreateProblemAction(const InputParameters & parameters)
  : MooseObjectAction(parameters)
{
}

void
SubChannelCreateProblemAction::act()
{
  // build the problem only if we have mesh
  if (_mesh.get() != NULL)
  {
    // when this action is built by parser with Problem input block, this action
    // must act i.e. create a problem. Thus if a problem has been created, it will error out.
    if (_problem)
      mooseError("Trying to build a problem but problem has already existed");

    _moose_object_pars.set<MooseMesh *>("mesh") = _mesh.get();
    _moose_object_pars.set<bool>("use_nonlinear") = _app.useNonlinear();

    _problem =
        _factory.create<FEProblemBase>(_type, getParam<std::string>("name"), _moose_object_pars);
  }
}
