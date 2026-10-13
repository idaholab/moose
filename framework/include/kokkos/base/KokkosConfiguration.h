//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosVariable.h"

class Coupleable;
class InputParameters;

namespace Moose::Kokkos
{

/**
 * The configuration on which a Kokkos object poses its weak form.
 *
 * Kokkos objects integrate on the reference configuration. An object with use_displaced_mesh =
 * true is posed on the current configuration x = X + u(X) given by the [Mesh] displacements, and
 * the Datum composes the deformation gradient F = I + grad_X u into its geometry:
 *
 *   dv = J dV,  grad_x = F^{-T} grad_X,  n da = J F^{-T} N dA,
 *
 * so that the object sees current-configuration quantities. For an isoparametric displacement
 * field this reproduces the quadrature of the same integrand on a mesh whose nodes are moved by u.
 */
class Configuration
{
public:
  /**
   * Default constructor, describing the reference configuration
   */
  Configuration() = default;
  /**
   * Constructor
   * @param object The object, which couples the displacements through the private parameter
   * _displacements when it is posed on the current configuration
   */
  Configuration(const Coupleable & object);

  /**
   * Get whether the object is posed on the current configuration
   * @returns Whether the object is posed on the current configuration
   */
  KOKKOS_FUNCTION bool displaced() const { return _displacements.coupled(); }
  /**
   * Get the displacement variable, whose component c displaces the reference position along
   * coordinate direction c
   * @returns The displacement variable
   */
  KOKKOS_FUNCTION const Variable & displacements() const { return _displacements; }
  /**
   * Get the displacement component discretized by a variable
   * @param sys The system number of the variable
   * @param var The variable number
   * @returns The displacement component, or libMesh::invalid_uint if the variable is not a
   * displacement
   */
  KOKKOS_FUNCTION unsigned int component(const unsigned int sys, const unsigned int var) const
  {
    if (displaced())
      for (unsigned int c = 0; c < _displacements.components(); ++c)
        if (_displacements.sys(c) == sys && _displacements.var(c) == var)
          return c;

    return libMesh::invalid_uint;
  }

private:
  /**
   * Displacements of the current configuration, uncoupled for the reference configuration
   */
  Variable _displacements;
};

/**
 * Add the parameters through which an object selects and couples its configuration:
 * use_displaced_mesh, and the private coupled parameter _displacements that the problem fills with
 * the [Mesh] displacements when use_displaced_mesh is true
 * @param params The parameters of the object
 */
void addConfigurationParams(InputParameters & params);

} // namespace Moose::Kokkos
