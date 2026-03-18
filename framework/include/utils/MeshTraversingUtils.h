//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/elem.h"
#include "libmesh/point.h"

namespace MeshTraversingUtils
{
/**
 * Determines whether the given element's subdomain id is in the given subdomain_id_list.
 * @param elem pointer to the element to consider
 * @param subdomain_id_list vector of subdomains to consider
 */
inline bool
elementSubdomainIdInList(const Elem * const elem,
                         const std::vector<subdomain_id_type> & subdomain_id_list)
{
  return std::find(subdomain_id_list.begin(), subdomain_id_list.end(), elem->subdomain_id()) !=
         subdomain_id_list.end();
}

/**
 * Determines whether two normal vectors are within normal_tol of each other.
 * @param normal_1 The first normal vector to compare to normal_2.
 * @param normal_2 The second normal vector to compare to normal_1.
 * @param tol The comparison tolerance.
 * @return A bool indicating whether 1 - dot(normal_1, normal_2) <= tol.
 */
inline bool
normalsWithinTol(const Point & normal_1, const Point & normal_2, const Real tol)
{
  return (1.0 - normal_1 * normal_2) <= tol;
}
}
