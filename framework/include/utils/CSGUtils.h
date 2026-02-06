//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CSGBase.h"

namespace CSGUtils
{

/**
 * Get inner region of given surfaces, defined as the intersection
 * of halfspaces of each surface. Here, the halfspace direction is determined based on the origin.
 * @param surfaces List of references to surfaces used to define inner region. This should
 * ideally be defined with the minimum number of surfaces needed to enclose the region
 * @param origin Point used to determine halfspace direction when defining intersected region
 * @return inner region defined by surfaces
 */
CSG::CSGRegion
getInnerRegion(const std::vector<std::reference_wrapper<const CSG::CSGSurface>> & surfaces,
               const libMesh::Point & origin);

/**
 * Check name of CSG component for disallowed characters and symbols
 *
 * @param name name of CSG component to check
 */
void checkValidCSGName(const std::string & name);
}
