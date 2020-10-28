//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADObjectTracker.h"

registerMooseObject("NavierStokesApp", INSADObjectTracker);

InputParameters
INSADObjectTracker::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription("User object used to track the kernels added to an INS simulation "
                             "and determine what properties to calculate in INSADMaterial");
  return params;
}

InputParameters
INSADObjectTracker::validTrackerParams()
{
  InputParameters params = emptyInputParameters();
  params.addParam<bool>("integrate_p_by_parts", "Whether to integrate the pressure by parts");
  MooseEnum viscous_form("traction laplace", "laplace");
  params.addParam<MooseEnum>("viscous_form",
                             viscous_form,
                             "The form of the viscous term. Options are 'traction' or 'laplace'");
  params.addParam<bool>(
      "has_boussinesq", false, "Whether the simulation has the boussinesq approximation");
  params.addParam<MaterialPropertyName>(
      "alpha", "The alpha material property for the boussinesq approximation");
  params.addParam<MaterialPropertyName>("ref_temp", "The reference temperature material property");
  params.addParam<std::string>("temperature", "The temperature variable");
  params.addParam<RealVectorValue>("gravity", "Direction of the gravity vector");
  params.addParam<bool>(
      "has_gravity",
      false,
      "Whether the simulation has a gravity force imposed on the momentum equation");
  params.addParam<bool>("has_transient", false, "Whether the momentum equations are transient");
  params.addParam<bool>("has_energy_transient", false, "Whether the energy equation is transient");

  addAmbientConvectionParams(params);

  params.addParam<bool>(
      "has_heat_source", false, "Whether there is a heat source function object in the simulation");
  params.addParam<FunctionName>("heat_source_function", "The function describing the heat source");
  params.addParam<std::string>(
      "heat_source_var",
      "Variable describing the volumetric heat source. Note that if this variable evaluates to a "
      "negative number, then this object will be an energy sink");

  params.addParam<bool>(
      "has_coupled_force",
      false,
      "Whether the simulation has a force due to a coupled vector variable/vector function");
  params.addParam<std::vector<VariableName>>("coupled_force_var",
                                             "Variables imposing coupled forces");
  params.addParam<std::vector<FunctionName>>("coupled_force_vector_function",
                                             "The function(s) standing in as a coupled force(s)");
  return params;
}

INSADObjectTracker::INSADObjectTracker(const InputParameters & parameters)
  : GeneralUserObject(parameters)
{
}

void
addAmbientConvectionParams(InputParameters & params)
{
  params.addParam<bool>(
      "has_ambient_convection",
      false,
      "Whether for the energy equation there is a heat source/sink due to convection "
      "from ambient surroundings");
  params.addParam<Real>("ambient_convection_alpha",
                        "The heat transfer coefficient from from ambient surroundings");
  params.addParam<Real>("ambient_temperature", "The ambient temperature");
}

template <>
bool
INSADObjectTracker::notEqual(const MooseEnum & val1, const MooseEnum & val2)
{
  return !val1.compareCurrent(val2);
}

void
INSADObjectTracker::addBlockIDs(const std::set<SubdomainID> & additional_block_ids)
{
  for (const auto sub_id : additional_block_ids)
    _block_id_to_params.emplace(sub_id, validTrackerParams());

  if (_block_id_to_params.find(Moose::ANY_BLOCK_ID) != _block_id_to_params.end() &&
      _block_id_to_params.size() > 1)
    mooseError(
        "If INSADObjectTracker::_block_id_to_params holds Moose::ANY_BLOCK_ID, then it cannot hold "
        "any other other block ids. Do you have an INSADMaterial that's not block "
        "restricted, and other INSADMaterials that are? If so, you have duplicate coverage, "
        "and you should fix that in your input file.");
}

bool
INSADObjectTracker::singleMaterialCoverage() const
{
  if (_block_id_to_params.find(Moose::ANY_BLOCK_ID) != _block_id_to_params.end())
  {
    mooseAssert(_block_id_to_params.size() == 1,
                "If our _block_id_to_params container has a Moose::ANY_BLOCK_ID key, then its size "
                "must be exactly one.");

    return true;
  }
  else
    return false;
}

bool
INSADObjectTracker::isTrackerParamValid(const std::string & name, const SubdomainID sub_id) const
{
  return getParams(sub_id).isParamValid(name);
}

const InputParameters &
INSADObjectTracker::getParams(const SubdomainID sub_id) const
{
  // Do we have a single unrestricted material that supplies all our material properties?
  if (singleMaterialCoverage())
    return _block_id_to_params.begin()->second;
  else
  {
    auto map_it = _block_id_to_params.find(sub_id);
    if (map_it == _block_id_to_params.end())
      mooseError("The requested sub_id is not a key in INSADObjectTracker::_block_id_to_params. "
                 "Please contact a Moose developer to fix this bug.");

    return map_it->second;
  }
}
