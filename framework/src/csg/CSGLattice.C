//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGLattice.h"
#include "CSGUtils.h"

namespace CSG
{

CSGLattice::CSGLattice(const std::string & name,
                       const std::string & lattice_type,
                       const std::optional<OuterVariant> & outer)
  : _name(name), _lattice_type(lattice_type), _outer_type("VOID"), _outer_universe(nullptr)
{
  CSGUtils::checkValidCSGName(name);
  // Handle the outer variant if provided
  if (outer.has_value())
  {
    std::visit([this](auto && outer_arg) { updateOuter(outer_arg); }, outer.value());
  }
}

void
CSGLattice::setUniverseAtIndex(const CSGUniverse & universe, const std::pair<int, int> index)
{
  std::string base_msg = "Cannot set universe at location (" + std::to_string(index.first) + ", " +
                         std::to_string(index.second) + ") for lattice " + getName() + ". ";
  if (_universe_map.size() == 0)
    mooseError(base_msg + "Universe map has not been initialized.");
  if (!isValidIndex(index))
    mooseError(base_msg + "Not a valid location.");
  _universe_map[index.first][index.second] = universe;
}

bool
CSGLattice::hasUniverse(const std::string & name) const
{
  for (auto list_univ : _universe_map)
    for (const CSGUniverse & univ : list_univ)
      if (univ.getName() == name)
        return true;
  return false; // no universe with matching name was found
}

const std::vector<std::vector<std::string>>
CSGLattice::getUniverseNameMap() const
{
  std::vector<std::vector<std::string>> name_map;
  for (auto & univ_list : _universe_map)
  {
    std::vector<std::string> name_list;
    for (const CSGUniverse & univ : univ_list)
      name_list.push_back(univ.getName());
    name_map.push_back(name_list);
  }
  return name_map;
}

const CSGUniverse &
CSGLattice::getUniverseAtIndex(const std::pair<int, int> index)
{
  if (!isValidIndex(index))
    mooseError("Index (" + std::to_string(index.first) + ", " + std::to_string(index.second) +
               ") is not a valid index for lattice " + getName());
  else
    return _universe_map[index.first][index.second];
}

const std::vector<std::pair<unsigned int, unsigned int>>
CSGLattice::getUniverseIndices(const std::string & univ_name) const
{
  if (!hasUniverse(univ_name))
    mooseError("Universe " + univ_name + " does not exist in lattice " + getName());

  std::vector<std::pair<unsigned int, unsigned int>> indices;
  for (auto i : index_range(_universe_map))
    for (auto j : index_range(_universe_map[i]))
    {
      const CSGUniverse & univ = _universe_map[i][j];
      if (univ.getName() == univ_name)
        indices.push_back(std::make_pair(i, j));
    }
  return indices;
}

const std::vector<std::reference_wrapper<const CSGUniverse>>
CSGLattice::getUniqueUniverses()
{
  std::vector<std::reference_wrapper<const CSGUniverse>> unique_univs;
  auto all_univs = getUniverses();

  for (const auto & ulist : all_univs)
    for (const auto & u : ulist)
    {
      auto it = std::find_if(unique_univs.begin(),
                             unique_univs.end(),
                             [&u](const auto & ref) { return &ref.get() == &u.get(); });
      if (it == unique_univs.end())
        unique_univs.push_back(u);
    }
  return unique_univs;
}

const CSGUniverse &
CSGLattice::getOuterUniverse() const
{
  if (getOuterType() != "UNIVERSE")
    mooseError("Lattice '" + getName() + "' has " + getOuterType() + " outer, not UNIVERSE.");
  else
    return *_outer_universe;
}

const std::string &
CSGLattice::getOuterMaterial() const
{
  if (getOuterType() != "CSG_MATERIAL")
    mooseError("Lattice '" + getName() + "' has " + getOuterType() + " outer, not CSG_MATERIAL.");
  else
    return _outer_material;
}

void
CSGLattice::updateOuter(const CSGUniverse & outer_universe)
{
  _outer_type = "UNIVERSE";
  _outer_universe = &outer_universe;
  _outer_material = "";
}

void
CSGLattice::updateOuter(const std::string & outer_name)
{
  _outer_type = "CSG_MATERIAL";
  _outer_material = outer_name;
  _outer_universe = nullptr;
}

void
CSGLattice::resetOuter()
{
  _outer_type = "VOID";
  _outer_material = "";
  _outer_universe = nullptr;
}

bool
CSGLattice::operator==(const CSGLattice & other) const
{
  if (this->getName() != other.getName())
    return false;
  if (this->getType() != other.getType())
    return false;
  if (this->getOuterType() != other.getOuterType())
    return false;
  if ((this->getOuterType() == "CSG_MATERIAL") &&
      (this->getOuterMaterial() != other.getOuterMaterial()))
    return false;
  if ((this->getOuterType() == "UNIVERSE") &&
      (this->getOuterUniverse() != other.getOuterUniverse()))
    return false;
  if (!this->compareAttributes(other))
    return false;

  const auto & this_univs = this->getUniverses();
  const auto & other_univs = other.getUniverses();
  if (this_univs.size() != other_univs.size())
    return false;
  for (unsigned int i = 0; i < this_univs.size(); ++i)
  {
    if (this_univs[i].size() != other_univs[i].size())
      return false;
    for (unsigned int j = 0; j < this_univs[i].size(); j++)
      if (this_univs[i][j].get() != other_univs[i][j].get())
        return false;
  }

  return true;
}

bool
CSGLattice::operator!=(const CSGLattice & other) const
{
  return !(*this == other);
}

void
CSGLattice::applyTransformation(TransformationType type,
                                const std::tuple<Real, Real, Real> & values)
{
  // Assert valid input as a safety measure
  // Main validation is done in CSGBase::applyTransformation
  mooseAssert(isValidTransformationValue(type, values),
              "Invalid transformation values for transformation type " +
                  getTransformationTypeString(type) + " on lattice " + getName());
  _transformations.emplace_back(type, values);
}

} // namespace CSG
