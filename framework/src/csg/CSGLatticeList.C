//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGLatticeList.h"

namespace CSG
{

CSGLatticeList::CSGLatticeList() {}

CSGLattice &
CSGLatticeList::getLattice(const std::string & name) const
{
  auto lat = _lattices.find(name);
  if (lat == _lattices.end())
    mooseError("No lattice by name " + name + " exists in the geometry.");
  else
    return *(lat->second);
}

std::vector<std::reference_wrapper<const CSGLattice>>
CSGLatticeList::getAllLattices() const
{
  std::vector<std::reference_wrapper<const CSGLattice>> lattices;
  for (auto it = _lattices.begin(); it != _lattices.end(); ++it)
    lattices.push_back(*(it->second.get()));
  return lattices;
}

CSGLattice &
CSGLatticeList::addLattice(std::unique_ptr<CSGLattice> lattice)
{
  auto name = lattice->getName();
  auto [it, inserted] = _lattices.emplace(name, std::move(lattice));
  if (!inserted)
    mooseError("Lattice with name " + name + " already exists in geometry.");
  return *it->second;
}

void
CSGLatticeList::renameLattice(const CSGLattice & lattice, const std::string & name)
{
  // check that this lattice passed in is actually in the same lattice that is in the lattice list
  auto prev_name = lattice.getName();
  auto it = _lattices.find(prev_name);
  if (it == _lattices.end() || it->second.get() != &lattice)
    mooseError("Lattice " + prev_name + " cannot be renamed to " + name +
               " as it does not exist in this CSGBase instance.");

  auto existing_lat = std::move(it->second);
  existing_lat->setName(name);
  _lattices.erase(prev_name);
  addLattice(std::move(existing_lat));
}

} // namespace CSG
