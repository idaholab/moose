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

#include "MooseTypes.h"
#include "MooseVariableFieldBase.h"
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
   * Number of components
   */
  unsigned int _components = 1;
  /**
   * Vector tag ID
   */
  TagID _tag = Moose::INVALID_TAG_ID;
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
};

} // namespace Moose::Kokkos
