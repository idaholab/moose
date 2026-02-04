//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGCell.h"
#include "CSGUniverse.h"
#include "CSGLattice.h"
#include "CSGUtils.h"

namespace CSG
{

CSGCell::CSGCell(const std::string & name, const CSGRegion & region)
  : _name(name), _fill_name(""), _region(region)
{
  CSGUtils::checkValidCSGName(name);
  _fill_type = "VOID";
}

CSGCell::CSGCell(const std::string & name, const std::string & mat_name, const CSGRegion & region)
  : _name(name), _fill_name(mat_name), _region(region)
{
  CSGUtils::checkValidCSGName(name);
  _fill_type = "CSG_MATERIAL";
}

CSGCell::CSGCell(const std::string & name, const CSGUniverse * univ, const CSGRegion & region)
  : _name(name), _fill_name(""), _region(region), _fill_universe(univ)
{
  CSGUtils::checkValidCSGName(name);
  _fill_type = "UNIVERSE";
}

CSGCell::CSGCell(const std::string & name, const CSGLattice * lattice, const CSGRegion & region)
  : _name(name), _fill_name(""), _region(region), _fill_lattice(lattice)
{
  CSGUtils::checkValidCSGName(name);
  _fill_type = "LATTICE";
}

const std::string &
CSGCell::getFillName() const
{
  if (getFillType() == "UNIVERSE")
    return _fill_universe->getName();
  else if (getFillType() == "LATTICE")
    return _fill_lattice->getName();
  else
    return _fill_name;
}

const CSGUniverse &
CSGCell::getFillUniverse() const
{
  if (getFillType() != "UNIVERSE")
    mooseError("Cell '" + getName() + "' has " + getFillType() + " fill, not UNIVERSE.");
  else
    return *_fill_universe;
}

const std::string &
CSGCell::getFillMaterial() const
{
  if (getFillType() != "CSG_MATERIAL")
    mooseError("Cell '" + getName() + "' has " + getFillType() + " fill, not CSG_MATERIAL.");
  else
    return _fill_name;
}

const CSGLattice &
CSGCell::getFillLattice() const
{
  if (getFillType() != "LATTICE")
    mooseError("Cell '" + getName() + "' has " + getFillType() + " fill, not LATTICE.");
  else
    return *_fill_lattice;
}

void
CSGCell::applyTransformation(TransformationType type, const std::vector<Real> & values)
{
  // Assert valid input as a safety measure
  // Main validation is done in CSGBase::applyTransformation
  mooseAssert(isValidTransformationValue(type, values),
              "Invalid transformation values for transformation type " +
                  getTransofrmationTypeString(type) + " on cell " + getName());
  _transformations.emplace_back(type, values);
}

bool
CSGCell::operator==(const CSG::CSGCell & other) const
{
  const auto name_eq = this->getName() == other.getName();
  const auto region_eq = this->getRegion() == other.getRegion();
  const auto fill_type_eq =
      (this->getFillType() == other.getFillType()) && (this->getFillName() == other.getFillName());
  if (name_eq && region_eq && fill_type_eq)
  {
    if (this->getFillType() == "CSG_MATERIAL")
      return this->getFillMaterial() == other.getFillMaterial();
    else if (this->getFillType() == "UNIVERSE")
      return this->getFillUniverse() == other.getFillUniverse();
    else if (this->getFillType() == "LATTICE")
      return this->getFillLattice() == other.getFillLattice();
    else
      return true;
  }
  else
    return false;
}

bool
CSGCell::operator!=(const CSG::CSGCell & other) const
{
  return !(*this == other);
}

} // namespace CSG
