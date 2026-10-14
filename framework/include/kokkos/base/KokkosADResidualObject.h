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
                   const Variable & displacements);

  /**
   * Whether a variable carries derivatives: a nonlinear variable at its current state, which for a
   * residual object must also belong to the residual system and be coupled to the residual
   * variable by the coupling matrix
   * @param ivar The Kokkos residual variable, or nullptr for a material
   */
  bool seeded(const Variable * ivar, const Variable & var, unsigned int comp = 0) const;

  /// Registered accessors
  std::vector<Request> _requests;
  /// Slot table built from the registered accessors
  ADSlotTable _table;
};

} // namespace Moose::Kokkos
