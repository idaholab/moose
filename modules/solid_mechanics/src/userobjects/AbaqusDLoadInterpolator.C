//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AbaqusDLoadInterpolator.h"
#include "AbaqusUELMesh.h"
#include "AbaqusUELStepUserObject.h"

registerMooseObject("SolidMechanicsApp", AbaqusDLoadInterpolator);

InputParameters
AbaqusDLoadInterpolator::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription("Interpolates Abaqus *Dload values at the beginning of each timestep "
                             "and stores per-element JDLTYP/ADLMAG arrays.");
  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_BEGIN};
  params.addParam<UserObjectName>("step_user_object",
                                  UserObjectName("step_uo"),
                                  "Step user object providing begin/end DLOAD maps.");
  return params;
}

AbaqusDLoadInterpolator::AbaqusDLoadInterpolator(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _uel_mesh(
        [this]() -> AbaqusUELMesh &
        {
          auto * mesh = dynamic_cast<AbaqusUELMesh *>(&_fe_problem.mesh());
          if (!mesh)
            mooseError("AbaqusDLoadInterpolator requires an AbaqusUELMesh.");
          return *mesh;
        }()),
    _step_uo(getUserObject<AbaqusUELStepUserObject>("step_user_object"))
{
}

void
AbaqusDLoadInterpolator::timestepSetup()
{
  _types.clear();
  _magnitudes.clear();

  // For each element in the model, merge begin/end loads by face and interpolate by step fraction
  const Real d = _step_uo.getStepFraction();
  const auto & elems = _uel_mesh.getElements();
  for (const auto idx : index_range(elems))
  {
    const auto * begin_list = _step_uo.getBeginDLoads(idx);
    const auto * end_list = _step_uo.getEndDLoads(idx);
    if (!begin_list && !end_list)
      continue;

    std::unordered_map<int, std::pair<Real, Real>> by_face; // begin, end
    if (begin_list)
      for (const auto & dl : *begin_list)
        by_face[dl._jdltyp].first = dl._magnitude;
    if (end_list)
      for (const auto & dl : *end_list)
        by_face[dl._jdltyp].second = dl._magnitude;

    std::vector<int> types;
    std::vector<Real> mags;
    types.reserve(by_face.size());
    mags.reserve(by_face.size());
    for (const auto & kv : by_face)
    {
      const int face = kv.first;
      const Real mnow = (1.0 - d) * kv.second.first + d * kv.second.second;
      if (mnow != 0.0)
      {
        types.push_back(face);
        mags.push_back(mnow);
      }
    }

    if (!types.empty())
    {
      _types[idx] = std::move(types);
      _magnitudes[idx] = std::move(mags);
    }
  }
}

const std::vector<int> *
AbaqusDLoadInterpolator::getTypes(Abaqus::Index elem_index) const
{
  const auto it = _types.find(elem_index);
  return it == _types.end() ? nullptr : &it->second;
}

const std::vector<Real> *
AbaqusDLoadInterpolator::getMagnitudes(Abaqus::Index elem_index) const
{
  const auto it = _magnitudes.find(elem_index);
  return it == _magnitudes.end() ? nullptr : &it->second;
}
