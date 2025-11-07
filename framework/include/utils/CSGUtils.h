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
 * @brief Helper function to convert std::any to JSON type
 *
 * @param data std::any object to convert
 * @return nlohmann::json representation of the data
 */
nlohmann::json anyToJson(const std::any & data);
}
