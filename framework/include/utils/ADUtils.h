//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseError.h"
#include "MooseTypes.h"
#include "MooseConfig.h"

class SystemBase;
namespace libMesh
{
class Elem;
}

namespace Moose
{
/**
 * Whether we are using global AD indexing
 */
inline bool
globalADIndexing()
{
  return true;
}

/**
 * Helper function for computing automatic differentiation offset. Let's explain how our derivative
 * index numbering scheme works:
 *
 * Let's just think about continuous finite elements for a second. We use a variable major
 * numbering scheme, such that each variables indices are in a contiguous block. Let's imagine we
 * have two variables, u and v, and we're on a \p QUAD4. The AD indices will be ordered like this:
 *
 * u0, u1, u2, u3, v0, v1, v2, v3
 *
 * \p max_dofs_per_elem should be for a \p QUAD4: 4. For a \p QUAD9, 9. \p HEX27, 27. Etc. For
 * CFEM the offset will be simply be the \p max_dofs_per_elem number times the \p var_num. So for u
 * on a \p QUAD4: 4 * 0 = 0. For v: 4 * 1. So u indices start at index 0, v indices start at
 * index 4.
 *
 * With DFEM or interface kernels it's a little more complicated. We essentially already have an
 * indices block that is \p num_vars_in_system * \p max_dofs_per_elem long, so in our two var,
 * \p QUAD4 example: 4 * 2 = 8. So what we do is that if we are on a neighbor element, we do an
 * additional offset by \p num_vars_in_system * \p max_dofs_per_elem. So now our derivative indices
 * are ordered like this:
 *
 * u0, u1, u2, u3, v0, v1, v2, v3, u0_neighbor, u1_neighbor, u2_neighbor, u3_neighbor, v0_neighbor,
 * v1_neighbor, v2_neighbor, v3_neighbor
 *
 * Finally if a lower-dimensional element is involved, then we another offset of \p
 * num_vars_in_system * \p max_dofs_per_elem:
 *
 * u0, u1, u2, u3, v0, v1, v2, v3, u0_neighbor, u1_neighbor, u2_neighbor, u3_neighbor, v0_neighbor,
 * v1_neighbor, v2_neighbor, v3_neighbor, u0_lower, u1_lower, u2_lower, u3_lower, v0_lower,
 * v1_lower, v2_lower, v3_lower
 *
 * Note that a lower dimensional block will have less indices than a higher dimensional one, but we
 * do not optimize for that consideration at this time
 *
 * @param var_num The variable number we are calculating the offset for
 * @param max_dofs_per_elem The maximum number of degrees of freedom for any one variable on an
 * element
 * @param element_type The "type" of element that we are on. Current options are
 * \p ElementType::Element, \p ElementType::Neighbor, and \p ElementType::Lower
 * @param num_vars_in_system The number of vars in the system. This is used in offset calculation
 * unless \p element_type is \p ElementType::Element
 * @return The automatic differentiation indexing offset
 *
 */
inline std::size_t
adOffset(unsigned int var_num,
         std::size_t max_dofs_per_elem,
         ElementType element_type = ElementType::Element,
         unsigned int num_vars_in_system = 0)
{
  // If our element type is anything other than ElementType::Element, then the user must
  // supply num_vars_in_system in order to calculate the offset
  mooseAssert(element_type == ElementType::Element || num_vars_in_system,
              "If our element type is anything other than ElementType::Element, then you "
              "must supply num_vars_in_system in order to calculate the offset");

  switch (element_type)
  {
    case ElementType::Element:
      return var_num * max_dofs_per_elem;

    case ElementType::Neighbor:
      return num_vars_in_system * max_dofs_per_elem + var_num * max_dofs_per_elem;

    case ElementType::Lower:
      return 2 * num_vars_in_system * max_dofs_per_elem + var_num * max_dofs_per_elem;

    default:
      mooseError(
          "Unsupported element type ",
          static_cast<typename std::underlying_type<decltype(element_type)>::type>(element_type));
  }
}

inline std::size_t
adOffset(unsigned int var_num,
         std::size_t max_dofs_per_elem,
         DGJacobianType dg_jacobian_type,
         unsigned int num_vars_in_system = 0)
{
  if (dg_jacobian_type == DGJacobianType::ElementElement ||
      dg_jacobian_type == DGJacobianType::NeighborElement)
    return adOffset(var_num, max_dofs_per_elem, ElementType::Element);
  else
    return adOffset(var_num, max_dofs_per_elem, ElementType::Neighbor, num_vars_in_system);
}

/**
 * Generate a map from global dof index to derivative value
 */
std::unordered_map<dof_id_type, Real>
globalDofIndexToDerivative(const ADReal & ad_real,
                           const SystemBase & sys,
                           ElementType elem_type = ElementType::Element,
                           THREAD_ID tid = 0);

/**
 * Generate a map from global dof index to derivative value for a (probably quadrature-point-based)
 * container like a material property or a variable value
 */
template <typename T>
auto
globalDofIndexToDerivative(const T & ad_real_container,
                           const SystemBase & sys,
                           ElementType elem_type = ElementType::Element,
                           THREAD_ID tid = 0)
    -> std::vector<std::unordered_map<
        dof_id_type,
        typename std::enable_if<std::is_same<ADReal, typename T::value_type>::value, Real>::type>>
{
  std::vector<std::unordered_map<dof_id_type, Real>> ret_val(ad_real_container.size());

  for (MooseIndex(ad_real_container) i = 0; i < ad_real_container.size(); ++i)
    ret_val[i] = globalDofIndexToDerivative(ad_real_container[i], sys, elem_type, tid);

  return ret_val;
}

}
