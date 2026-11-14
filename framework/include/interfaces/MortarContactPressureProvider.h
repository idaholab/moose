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

class MooseObject;
class MortarUserObject;

namespace libMesh
{
class Elem;
class Node;
}

/**
 * Interface for objects that provide the physical normal contact pressure of a mortar interface.
 * A contact Lagrange multiplier variable can store a scaled pressure, so objects in other modules
 * that need the pressure obtain it from the contact object through this interface.
 */
class MortarContactPressureProvider
{
public:
  /**
   * @param provider_object The mortar user object that implements this interface. Consumers must
   * share its interface and mesh.
   */
  MortarContactPressureProvider(const MortarUserObject & provider_object);

  virtual ~MortarContactPressureProvider() = default;

  /**
   * @param node A secondary node of the mortar interface
   * @return The physical normal contact pressure at \p node, carrying its derivatives with
   * respect to the solution
   */
  virtual ADReal nodalContactPressure(const libMesh::Node & node) const = 0;

  /**
   * @return The basis that interpolates the nodal contact pressures on the current secondary
   * lower-dimensional element, evaluated at the current mortar segment quadrature points
   */
  virtual const VariableTestValue & tractionBasis() const = 0;

  /**
   * @param lower_secondary_elem The current secondary lower-dimensional element
   * @param qp The current mortar segment quadrature point
   * @return The physical normal contact pressure at \p qp, interpolated from the nodal pressures
   * of \p lower_secondary_elem with tractionBasis()
   */
  ADReal qpContactPressure(const libMesh::Elem & lower_secondary_elem, unsigned int qp) const;

  /**
   * Report an error unless a consumer lies on the same mortar interface and mesh as this provider.
   * nodalContactPressure() and tractionBasis() are only valid for the nodes and quadrature points
   * of that interface and mesh.
   * @param consumer The object that reads the contact pressure
   * @param param The consumer parameter that names this provider
   * @param primary_subdomain The consumer primary lower-dimensional subdomain
   * @param secondary_subdomain The consumer secondary lower-dimensional subdomain
   * @param use_displaced_mesh Whether the consumer operates on the displaced mesh
   */
  void checkConsumer(const MooseObject & consumer,
                     const std::string & param,
                     SubdomainID primary_subdomain,
                     SubdomainID secondary_subdomain,
                     bool use_displaced_mesh) const;

private:
  /// The mortar user object that implements this interface
  const MortarUserObject & _provider_object;
};
