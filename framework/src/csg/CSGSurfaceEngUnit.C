//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGSurfaceEngUnit.h"

namespace CSG
{

CSGSurfaceEngUnit::CSGSurfaceEngUnit(const std::string & name, const std::string & unit_type)
  : CSGSurface(name, unit_type), CSGEngUnit("SURFACE", unit_type)
{
}

std::unordered_map<std::string, Real>
CSGSurfaceEngUnit::getCoeffs() const
{
  mooseError("CSGSurfaceEngUnit '",
             getName(),
             "' must be expanded via CSGBase::expandEngUnit() before calling getCoeffs().");
}

} // namespace CSG
