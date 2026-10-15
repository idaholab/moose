//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosADVariableValue.h"

#include <type_traits>

namespace Moose::Kokkos
{

/**
 * Setup of the AD accessors of a pointwise AD object. The accessors are created during
 * construction and register their requests here. During initial setup, once the coupling matrix
 * of the residual system is final, each request whose variable is a nonlinear variable of the
 * residual system coupled to the residual variable receives a slot block, and every other request
 * carries no derivatives. A material has no residual variable: each request whose variable is a
 * nonlinear variable at its current state receives a slot block, and its consumers drop the
 * blocks their coupling excludes.
 */
class ADSlotSetup
{
public:
  /**
   * Register an AD accessor
   * @param slot The slot assignment of the accessor
   * @param var The variable read by the accessor
   * @param gradient Whether the accessor reads the gradient
   */
  void add(ADSlotAssignment & slot, const Variable & var, bool gradient);
  /**
   * Register an AD material property read by the object, whose stored derivatives are mapped to
   * the slots of the object
   * @param map The slot map of the reader
   * @param inputs The slot blocks of the stored derivatives in each subdomain
   */
  void addProperty(ADPropertySlotMap & map,
                   const std::unordered_map<SubdomainID, std::vector<ADSlotBlock>> & inputs);

  /**
   * Assign the slots of all registered accessors
   * @param object The residual object, for error messages
   * @param ivar The Kokkos residual variable
   * @param max_slots The number of derivative slots declared by the residual object
   */
  void assign(const MooseObject & object, const Variable & ivar, unsigned int max_slots);
  /**
   * Assign the slots of all registered accessors, followed by the displacement slots of the
   * current configuration
   * @param object The residual object, for error messages
   * @param ivar The Kokkos residual variable
   * @param max_slots The number of derivative slots declared by the residual object, which the
   * accessors may occupy ahead of the displacement slots
   * @param displacements The displacement variable of the current configuration
   */
  void assign(const MooseObject & object,
              const Variable & ivar,
              unsigned int max_slots,
              const Variable & displacements);
  /**
   * Assign the slots of all registered accessors and the slot maps of all registered AD material
   * properties, followed by the displacement slots of the current configuration or of the
   * properties that depend on the displacements
   * @param object The residual object, for error messages
   * @param ivar The Kokkos residual variable
   * @param max_slots The number of derivative slots declared by the residual object, which the
   * accessors and the property inputs may occupy ahead of the displacement slots
   * @param displacements The displacement variable of the current configuration
   * @param mesh The Kokkos mesh, which numbers the subdomains of the slot maps
   */
  void assign(const MooseObject & object,
              const Variable & ivar,
              unsigned int max_slots,
              const Variable & displacements,
              const Mesh & mesh);
  /**
   * Assign the slots of all registered accessors of a material, followed by the displacement slots
   * of the current configuration
   * @param object The material, for error messages
   * @param max_slots The number of derivative slots declared by the material, which the accessors
   * may occupy ahead of the displacement slots
   * @param displacements The displacement variable of the current configuration
   */
  void assign(const MooseObject & object, unsigned int max_slots, const Variable & displacements);

  /**
   * Get the slot table
   */
  const ADSlotTable & table() const { return _table; }

private:
  /// A registered accessor
  struct Request
  {
    /// Slot assignment written into the accessor
    ADSlotAssignment * slot;
    /// Variable read by the accessor
    Variable var;
    /// Whether the accessor reads the gradient
    bool gradient;
  };

  /**
   * Assign the slots of all registered accessors
   * @param object The object, for error messages
   * @param ivar The Kokkos residual variable, or nullptr for a material
   * @param max_slots The number of derivative slots declared by the object
   * @param displacements The displacement variable of the current configuration
   */
  void assignSlots(const MooseObject & object,
                   const Variable * ivar,
                   unsigned int max_slots,
                   const Variable & displacements,
                   const Mesh * mesh = nullptr);

  /**
   * Whether a variable carries derivatives: a nonlinear variable at its current state, which for a
   * residual object must also belong to the residual system and be coupled to the residual
   * variable by the coupling matrix
   * @param ivar The Kokkos residual variable, or nullptr for a material
   */
  bool seeded(const Variable * ivar, const Variable & var, unsigned int comp = 0) const;

  /// A registered AD material property
  struct PropertyRequest
  {
    /// Slot map written into the reader
    ADPropertySlotMap * map;
    /// Slot blocks of the stored derivatives in each subdomain
    const std::unordered_map<SubdomainID, std::vector<ADSlotBlock>> * inputs;
  };

  /**
   * Whether a variable carries derivatives for a residual object: a variable of the residual
   * system, all of which are nonlinear, that the coupling matrix couples to the residual variable
   * @param ivar The Kokkos residual variable
   * @param sys The system number of the variable
   * @param var The variable number in its system
   */
  bool seeded(const Variable & ivar, unsigned int sys, unsigned int var) const;
  /**
   * Assign the slot maps of the registered AD material properties
   * @param ivar The Kokkos residual variable
   * @param mesh The Kokkos mesh
   */
  void assignProperties(const Variable & ivar, const Mesh & mesh);

  /// Registered accessors
  std::vector<Request> _requests;
  /// Registered AD material properties
  std::vector<PropertyRequest> _property_requests;
  /// Slot table built from the registered accessors
  ADSlotTable _table;
};

} // namespace Moose::Kokkos
