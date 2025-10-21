//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGCartesianLattice.h"

namespace CSG
{

CSGCartesianLattice::CSGCartesianLattice(const std::string & name)
  : CSGLattice(name, MooseUtils::prettyCppType<CSGCartesianLattice>())
{
}

CSGCartesianLattice::CSGCartesianLattice(
    const std::string & name,
    const Real pitch,
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> universes)
  : CSGLattice(name, universes, MooseUtils::prettyCppType<CSGCartesianLattice>()),
    _nx0(_universe_map.size()),
    _nx1(_universe_map[0].size()),
    _pitch(pitch)
{
}

CSGCartesianLattice::CSGCartesianLattice(
    const std::string & name,
    const Real pitch,
    const int nx0,
    const int nx1,
  : CSGLattice(name, MooseUtils::prettyCppType<CSGCartesianLattice>()), _nx0(nx0), _nx1(nx1), _pitch(pitch)
{
  // create universe map of empty elements
}

virtual bool CSGCartesianLattice::hasValidDimensions()
{
  std::string base_msg = "Lattice " + getName() + " of type " + getType();
  mooseAssert(_pitch > 0, base_msg + " must have pitch > 0.");
  mooseAssert(_nx0 > 0, base_msg + " must have more than 0 elements in the first dimension (nx0).");
  mooseAssert(_nx1 > 0,
              base_msg + " must have more than 0 elements in the second dimension (nx1).");


}

} // namespace CSG
