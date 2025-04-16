//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AbaqusUELStateVariables.h"
#include "AbaqusUELMeshUserElement.h"
#include "Conversion.h"

registerMooseObject("SolidMechanicsApp", AbaqusUELStateVariables);

InputParameters
AbaqusUELStateVariables::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addRequiredParam<UserObjectName>("uel", "The AbaqusUELMeshUserElement user object name");
  params.addParam<std::vector<std::string>>(
      "column_names",
      {},
      "Custom column names. Names are applied in order starting at the first state variable. "
      "Custom names nit supplied will get automatically assigned names.");
  params.addRangeCheckedParam<unsigned int>(
      "split", 1, "split>0", "split the statevariables in this many sub sections");
  params.addClassDescription("Outputs the state variables for all UEL user elements");
  return params;
}

AbaqusUELStateVariables::AbaqusUELStateVariables(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _uel(getUserObject<AbaqusUELMeshUserElement>("uel")),
    _active_elements(_uel.getActiveElements()),
    _uel_elements(_uel.getElements()),
    _id_vector(declareVector("abaqus_elem_id")),
    _split(getParam<unsigned int>("split")),
    _split_vector(_split > 1 ? &declareVector("IntP") : nullptr)
{
  const auto names = getParam<std::vector<std::string>>("column_names");
  const auto nstatev = _uel.getStateVars().first;
  if (nstatev % _split != 0)
    paramError("split",
               "The number of state variables (",
               nstatev,
               ") is not evenly divisible by the specified number of splits.");

  const auto nbatch = nstatev / _split;
  for (const auto i : make_range(nbatch))
    _value_vectors.push_back(
        &declareVector(i < names.size() ? names[i] : "state_var_" + Moose::stringify(i + 1)));
}

void
AbaqusUELStateVariables::initialSetup()
{
}

void
AbaqusUELStateVariables::initialize()
{
  _id_vector.clear();
  if (_split_vector)
    _split_vector->clear();
  for (auto & vec : _value_vectors)
    vec->clear();
}

void
AbaqusUELStateVariables::execute()
{
  const auto & [nstatev, statev] = _uel.getStateVars();
  for (const auto i : _active_elements)
  {
    const auto abaqus_elem_id = _uel_elements[i]._id;
    const auto it = statev.find(abaqus_elem_id);
    if (it != statev.end() && it->second.size() == nstatev)
    {
      const auto nbatch = nstatev / _split;

      for (const auto i : make_range(_split))
      {
        _id_vector.push_back(abaqus_elem_id);
        if (_split_vector)
          _split_vector->push_back(i + 1);
        for (const auto j : make_range(nbatch))
          _value_vectors[j]->push_back(it->second[i * nbatch + j]);
      }
    }
  }
}

void
AbaqusUELStateVariables::finalize()
{
}

void
AbaqusUELStateVariables::threadJoin(const UserObject &)
{
}
