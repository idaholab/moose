//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosTypes.h"
#include "KokkosFunction.h"

#include "MooseTypes.h"
#include "MooseVariableBase.h"
#include "MoosePassKey.h"

class Coupleable;

namespace Moose::Kokkos
{

/**
 * The Kokkos variable object that carries the coupled variable and tag information
 */
class Variable
{
public:
  using CoupleableKey = ::Moose::PassKey<::Coupleable>;

  /**
   * Default constructor
   */
  Variable() = default;
  /**
   * Constructor
   * Initialize the variable with a MOOSE variable and vector tag ID
   * @param variable The MOOSE variable
   * @param tag The vector tag ID
   */
  Variable(const MooseVariableFieldBase & variable, const TagID tag) { init(variable, tag); }
  /**
   * Constructor
   * Initialize the variable with a MOOSE variable and vector tag name
   * @param variable The MOOSE variable
   * @param tag_name The vector tag name
   */
  Variable(const MooseVariableFieldBase & variable, const TagName & tag_name = Moose::SOLUTION_TAG)
  {
    init(variable, tag_name);
  }
  /**
   * Constructor
   * Initialize the variable with multiple MOOSE variables and vector tag ID
   * @param variables The MOOSE variables
   * @param tag The vector tag ID
   */
  ///@{
  Variable(const std::vector<const MooseVariableFieldBase *> & variables, const TagID tag)
  {
    init(variables, tag);
  }
  Variable(const std::vector<MooseVariableFieldBase *> & variables, const TagID tag)
  {
    init(variables, tag);
  }
  ///@}
  /**
   * Constructor
   * Initialize the variable with multiple MOOSE variables and vector tag name
   * @param variables The MOOSE variables
   * @param tag The vector tag ID
   */
  ///@{
  Variable(const std::vector<const MooseVariableFieldBase *> & variables,
           const TagName & tag_name = Moose::SOLUTION_TAG)
  {
    init(variables, tag_name);
  }
  Variable(const std::vector<MooseVariableFieldBase *> & variables,
           const TagName & tag_name = Moose::SOLUTION_TAG)
  {
    init(variables, tag_name);
  }
  ///@}
  /**
   * Initialize the variable with a MOOSE variable and vector tag ID
   * @param variable The MOOSE variable
   * @param tag The vector tag ID
   */
  void init(const MooseVariableFieldBase & variable, const TagID tag);
  /**
   * Initialize the variable with a MOOSE variable and vector tag name
   * @param variable The MOOSE variable
   * @param tag_name The vector tag name
   */
  void init(const MooseVariableFieldBase & variable,
            const TagName & tag_name = Moose::SOLUTION_TAG);
  /**
   * Initialize the variable with multiple MOOSE variables and vector tag ID
   * @param variables The MOOSE variables
   * @param tag The vector tag ID
   */
  ///@{
  void init(const std::vector<const MooseVariableFieldBase *> & variables, const TagID tag);
  void init(const std::vector<MooseVariableFieldBase *> & variables, const TagID tag);
  ///@}
  /**
   * Initialize the variable with multiple MOOSE variables and vector tag name
   * @param variables The MOOSE variables
   * @param tag_name The vector tag name
   */
  ///@{
  void init(const std::vector<const MooseVariableFieldBase *> & variables,
            const TagName & tag_name = Moose::SOLUTION_TAG);
  void init(const std::vector<MooseVariableFieldBase *> & variables,
            const TagName & tag_name = Moose::SOLUTION_TAG);
  ///@}
  /**
   * Initialize the variable with coupled default values
   * @param values The default coupled values
   */
  void init(const std::vector<Real> & values, CoupleableKey);

  /**
   * Get the MOOSE variable of a component
   * @param comp The variable component
   * @returns The MOOSE variable
   */
  const MooseVariableFieldBase * mooseVar(unsigned int comp = 0)
  {
    return _moose_var.size() ? _moose_var[comp] : nullptr;
  }

  /**
   * Get whether the variable is initialized
   * @returns Whether the variable is initialized
   */
  KOKKOS_FUNCTION bool initialized() const { return _initialized; }
  /**
   * Get whether the variable is coupled
   * @returns Whether the variable is coupled
   */
  KOKKOS_FUNCTION bool coupled() const { return _coupled; }
  /**
   * Get whether the variable is nodal
   * @returns Whether the variable is nodal
   */
  KOKKOS_FUNCTION bool nodal() const { return _nodal; }
  /**
   * Get whether the tag is time derivative
   * @returns Whether the tag is time derivative
   */
  KOKKOS_FUNCTION bool dot() const { return _dot; }
  /**
   * Get whether the tag is old/older value
   * @returns Whether the tag is old/older value
   */
  KOKKOS_FUNCTION bool old() const { return _old; }
  /**
   * Get the number of components
   * @returns The number of components
   */
  KOKKOS_FUNCTION unsigned int components() { return _components; }
  /**
   * Get the vector tag ID
   * @returns The vector tag ID
   */
  KOKKOS_FUNCTION TagID tag() const { return _tag; }
  /**
   * Get the variable number of a component
   * @param comp The variable component
   * @returns The variable number
   */
  KOKKOS_FUNCTION unsigned int var(unsigned int comp = 0) const { return _var[comp]; }
  /**
   * Get the system number of a component
   * @param comp The variable component
   * @returns The system number
   */
  KOKKOS_FUNCTION unsigned int sys(unsigned int comp = 0) const { return _sys[comp]; }
  /**
   * Get the default value of a component
   * @param comp The variable component
   * @returns The default value
   */
  KOKKOS_FUNCTION Real value(unsigned int comp = 0) const
  {
    KOKKOS_ASSERT(!_coupled);

    return _default_value[comp];
  }

  /**
   * Register boundary conditions for this variable, moving them to device.
   * Called from a flux kernel's initialSetup() after collecting BCs from the warehouse.
   * @param ids The boundary IDs covered by the BCs
   * @param values The function providing the BC value on each boundary
   */
  void initBoundaryConditions(const std::vector<BoundaryID> & ids,
                              const std::vector<Moose::Kokkos::Function> & values);

  /**
   * Return whether a BC is registered for the given boundary ID
   * @param id The boundary ID to query
   */
  KOKKOS_FUNCTION bool hasBoundaryCondition(BoundaryID id) const
  {
    return boundaryConditionIndex(id) >= 0;
  }

  /**
   * Return the index of the BC registered for the given boundary ID, or -1 if absent
   * @param id The boundary ID to query
   */
  KOKKOS_FUNCTION int boundaryConditionIndex(BoundaryID id) const
  {
    for (std::size_t i = 0; i < _boundary_ids.size(); ++i)
      if (_boundary_ids[i] == id)
        return static_cast<int>(i);
    return -1;
  }

  /**
   * Return the BC function at the given index (from boundaryConditionIndex())
   * @param index The BC index
   */
  KOKKOS_FUNCTION const Moose::Kokkos::Function & boundaryConditionValue(int index) const
  {
    return _boundary_values[index];
  }

private:
  /**
   * Whether the variable is initialized
   */
  bool _initialized = false;
  /**
   * Whether the variable is coupled
   */
  bool _coupled = false;
  /**
   * Whether the variable is nodal
   */
  bool _nodal = false;
  /**
   * Whether the tag is time derivative
   */
  bool _dot = false;
  /**
   * Whether the tag is old/older value
   */
  bool _old = false;
  /**
   * Number of components
   */
  unsigned int _components = 1;
  /**
   * Vector tag ID
   */
  TagID _tag = Moose::INVALID_TAG_ID;
  /**
   * MOOSE variable of each component
   */
  Array<const MooseVariableFieldBase *> _moose_var;
  /**
   * Variable number of each component
   */
  Array<unsigned int> _var;
  /**
   * System number of each component
   */
  Array<unsigned int> _sys;
  /**
   * Default value of each component when the variable is not coupled
   */
  Array<Real> _default_value;
  /**
   * Boundary IDs for which a BC has been registered (populated by initBoundaryConditions())
   */
  Array<BoundaryID> _boundary_ids;
  /**
   * BC function values indexed in parallel with _boundary_ids
   */
  Array<Moose::Kokkos::Function> _boundary_values;
};

} // namespace Moose::Kokkos
