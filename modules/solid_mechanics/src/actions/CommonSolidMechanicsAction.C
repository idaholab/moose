//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CommonSolidMechanicsAction.h"
#include "QuasiStaticSolidMechanicsPhysics.h"
#include "ActionWarehouse.h"
#include "FEProblemBase.h"
#include "Factory.h"

registerMooseAction("SolidMechanicsApp", CommonSolidMechanicsAction, "meta_action");

registerMooseAction("SolidMechanicsApp", CommonSolidMechanicsAction, "add_variable");

InputParameters
CommonSolidMechanicsAction::validParams()
{
  InputParameters params = QuasiStaticSolidMechanicsPhysicsBase::validParams();
  params.addClassDescription("Store common solid mechanics parameters");
  return params;
}

CommonSolidMechanicsAction::CommonSolidMechanicsAction(const InputParameters & parameters)
  : Action(parameters)
{
}

void
CommonSolidMechanicsAction::act()
{

  // check if sub-blocks block are found which will use the common parameters
  auto actions = _awh.getActions<QuasiStaticSolidMechanicsPhysicsBase>();
  if (actions.size() == 0)
    mooseWarning("Common parameters are supplied, but not used in ", parameters().blockLocation());

  // Add disp-variables
  if (_current_task == "add_variable")
  {
    // We set these variables first with the "common" (not nested) level parameters
    bool do_add_variables = getParam<bool>("add_variables");
    auto displacement_variables = getParam<std::vector<VariableName>>("displacements");
    bool add_variables_block_restricted = true;
    std::set<SubdomainName> add_variables_blocks;
    bool scaling_set = isParamValid("scaling");
    Real scaling_value = (scaling_set) ? getParam<Real>("scaling") : 1.0;

    // Check all nested sub-actions and update the variables keeping track of each selection
    for (const auto & action : actions)
    {
      if (action->getParam<bool>("add_variables"))
      {
        // this sub-action wants the disp-variables added.
        do_add_variables = true;
        const auto v = action->getParam<std::vector<VariableName>>("displacements");
        if (v.size() > 0)
        {
          if (displacement_variables.size() == 0)
            displacement_variables.insert(displacement_variables.end(), v.begin(), v.end());
          else if (displacement_variables != v)
            paramError("displacements",
                       "The vector of displacement variables of the actions differ.");
        }

        // with block-restriction?
        if (add_variables_block_restricted && action->isParamValid("block") &&
            action->getParam<std::vector<SubdomainName>>("block").size())
        {
          auto action_blocks = action->getParam<std::vector<SubdomainName>>("block");
          mooseAssert(action_blocks.size(), "Block restriction must not be empty");
          add_variables_blocks.insert(action_blocks.cbegin(), action_blocks.cend());
        }
        else
          add_variables_block_restricted = false;

        // scaling?
        if (action->getParam<bool>("add_variables") && action->isParamValid("scaling"))
        {
          const auto scaling = action->getParam<Real>("scaling");

          // sanity-check
          if (scaling_set && scaling_value != scaling)
            paramError("scaling", "The scaling parameter of the actions differ.");

          // set local scaling variables
          scaling_set = true;
          scaling_value = scaling;
        }
      }
    }

    // add the disp-variables, if desired
    if (do_add_variables)
    {
      auto params = _factory.getValidParams("MooseVariable");
      // determine necessary order
      const bool second = _problem->mesh().hasSecondOrderElements();

      params.set<MooseEnum>("order") = second ? "SECOND" : "FIRST";
      params.set<MooseEnum>("family") = "LAGRANGE";
      if (add_variables_block_restricted && add_variables_blocks.size() > 0)
      {
        std::vector<SubdomainName> blks(add_variables_blocks.begin(), add_variables_blocks.end());
        params.set<std::vector<SubdomainName>>("block") = blks;
      }
      if (scaling_set && scaling_value != 1)
        params.set<std::vector<Real>>("scaling") = {scaling_value};

      // Loop through the displacement variables
      const auto n = displacement_variables.size();
      if (n == 0)
        paramError("displacements", "The vector of displacement variables is not set.");
      if (n != _problem->mesh().dimension())
        paramError("displacements",
                   "The number of displacement variables differs from the number of dimensions of "
                   "the mesh.");

      for (const auto & disp : displacement_variables)
        // Create displacement variables
        _problem->addVariable("MooseVariable", disp, params);
    }
  }
}
