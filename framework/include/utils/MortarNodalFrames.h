//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "ADReal.h"

#include "libmesh/vector_value.h"
#include "libmesh/point.h"

#include <array>
#include <unordered_map>
#include <vector>

class AutomaticMortarGeneration;
class MooseBase;
class SystemBase;
template <typename>
class MooseVariableFE;
typedef MooseVariableFE<Real> MooseVariable;
namespace libMesh
{
class Elem;
class Node;
}

namespace Moose
{
namespace Mortar
{
/**
 * @param node A node of the displaced mesh
 * @param disp_vars The x, y, and z displacement variables; the z entry is null in 2D
 * @return The current coordinates of \p node, carrying a unit derivative with respect to each of
 * its displacement degrees of freedom
 */
ADPoint nodalCoordinate(const Node & node, const std::array<const MooseVariable *, 3> & disp_vars);

/**
 * @param disp_vars The x, y, and z displacement variables; the z entry is null in 2D
 * @param sys The nonlinear system assembled by the mortar object
 * @return Whether every displacement variable is nodal and belongs to \p sys, as nodal-normal
 * derivatives require
 */
bool supportsNodalNormalDerivatives(const std::array<const MooseVariable *, 3> & disp_vars,
                                    const SystemBase & sys);

/**
 * Error unless supportsNodalNormalDerivatives() holds
 * @param object The mortar object whose parameters name the displacement variables
 * @param disp_vars The x, y, and z displacement variables; the z entry is null in 2D
 * @param sys The nonlinear system assembled by \p object
 */
void checkNodalNormalDisplacements(const MooseBase & object,
                                   const std::array<const MooseVariable *, 3> & disp_vars,
                                   const SystemBase & sys);

/**
 * Cache of the secondary nodal normals and Householder tangents of one mortar interface, carrying
 * their displacement derivatives while a Jacobian is assembled. Residual-only evaluations hold the
 * stored nodal geometry values. Call \p clear() whenever the displaced geometry changes.
 */
class ADNodalFrames
{
public:
  /**
   * @param disp_vars The x, y, and z displacement variables; the z entry is null in 2D
   */
  explicit ADNodalFrames(const std::array<const MooseVariable *, 3> & disp_vars)
    : _disp_vars(disp_vars)
  {
  }

  /**
   * @param amg The mortar generation object of the interface
   * @param node A secondary node
   * @param stored_normal The stored normal at \p node, used when \p do_derivatives is false
   * @param do_derivatives Whether to include displacement derivatives
   * @return The unit normal at \p node
   */
  const ADRealVectorValue & normal(const AutomaticMortarGeneration & amg,
                                   const Node & node,
                                   const Point & stored_normal,
                                   bool do_derivatives);

  /**
   * @return The Householder tangents at \p node, consistent with \p normal()
   */
  const std::array<ADRealVectorValue, 2> & tangents(const AutomaticMortarGeneration & amg,
                                                    const Node & node,
                                                    const Point & stored_normal,
                                                    bool do_derivatives);

  /**
   * Interpolate the nodal normals of \p secondary_elem to reference points in the same way as
   * AutomaticMortarGeneration::getNormals()
   * @param amg The mortar generation object of the interface
   * @param secondary_elem The lower-dimensional secondary element
   * @param xi_pts The reference points on \p secondary_elem
   * @param stored_normals The stored interpolated normals at \p xi_pts, which also fix the sign
   * convention of a periodic interface
   * @param do_derivatives Whether to include displacement derivatives
   * @return The interpolated normals, one per point
   */
  std::vector<ADRealVectorValue> interpolatedNormals(const AutomaticMortarGeneration & amg,
                                                     const Elem & secondary_elem,
                                                     const std::vector<Point> & xi_pts,
                                                     const std::vector<Point> & stored_normals,
                                                     bool do_derivatives);

  void clear();

private:
  const std::array<const MooseVariable *, 3> _disp_vars;
  std::unordered_map<const Node *, ADRealVectorValue> _normals;
  std::unordered_map<const Node *, std::array<ADRealVectorValue, 2>> _tangents;
};
}
}
