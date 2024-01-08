//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CohesiveZoneMortarUserObjectAux.h"

// supported user objects
#include "BilinearMixedModeCohesiveZoneModel.h"

registerMooseObject("ContactApp", CohesiveZoneMortarUserObjectAux);

const MooseEnum CohesiveZoneMortarUserObjectAux::_cohesive_zone_quantities(
    "mode_mixity_ratio cohesive_damage local_normal_jump local_tangential_jump");

InputParameters
CohesiveZoneMortarUserObjectAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Populates an auxiliary variable with mortar cohesive zone model quantities.");
  params.addRequiredParam<MooseEnum>(
      "cohesive_zone_quantity",
      _cohesive_zone_quantities,
      "The desired cohesive zone model quantity to output as an auxiliary variable.");
  params.addRequiredParam<UserObjectName>("user_object",
                                          "The mortar cohesive zone modeling user object to get "
                                          "values from.  Note that the user object "
                                          "must implement the corresponding getter function.");
  params.set<ExecFlagEnum>("execute_on") = {EXEC_TIMESTEP_END};
  params.suppressParameter<ExecFlagEnum>("execute_on");
  return params;
}

CohesiveZoneMortarUserObjectAux::CohesiveZoneMortarUserObjectAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _cohesive_zone_quantity(
        getParam<MooseEnum>("cohesive_zone_quantity").getEnum<CohesiveQuantityEnum>()),
    _user_object(getUserObject<UserObject>("user_object")),
    _cohesive_zone_uo(dynamic_cast<const BilinearMixedModeCohesiveZoneModel *>(&_user_object)),
    _outputs({{CohesiveQuantityEnum::MODE_MIXITY_RATIO,
               {"BilinearMixedModeCohesiveZoneModel",
                _cohesive_zone_uo,
                [&]() { return _cohesive_zone_uo->getModeMixityRatio(_current_node); }}},
              {CohesiveQuantityEnum::COHESIVE_DAMAGE,
               {"BilinearMixedModeCohesiveZoneModel",
                _cohesive_zone_uo,
                [&]() { return _cohesive_zone_uo->getCohesiveDamage(_current_node); }}},
              {CohesiveQuantityEnum::LOCAL_NORMAL_JUMP,
               {"BilinearMixedModeCohesiveZoneModel",
                _cohesive_zone_uo,
                [&]() { return _cohesive_zone_uo->getLocalDisplacementNormal(_current_node); }}},
              {CohesiveQuantityEnum::LOCAL_TANGENTIAL_JUMP,
               {"BilinearMixedModeCohesiveZoneModel", _cohesive_zone_uo, [&]() {
                  return _cohesive_zone_uo->getLocalDisplacementTangential(_current_node);
                }}}})
{
  if (!isNodal())
    mooseError("This auxiliary kernel requires nodal variables to obtain contact pressure values");

  // error check
  const auto it = _outputs.find(_cohesive_zone_quantity);
  if (it == _outputs.end())
    mooseError("Internal error: Contact quantity request in PressureMortarUserObjectAux is not "
               "recognized.");
  if (!std::get<1>(it->second))
    paramError("user_object",
               "The '",
               _cohesive_zone_quantities.getNames()[static_cast<int>(it->first)],
               "' quantity is only provided by a '",
               std::get<0>(it->second),
               "' or derived object.");
}

Real
CohesiveZoneMortarUserObjectAux::computeValue()
{
  // execute functional to retrieve selected quantity
  return std::get<2>(_outputs[_cohesive_zone_quantity])();
}
