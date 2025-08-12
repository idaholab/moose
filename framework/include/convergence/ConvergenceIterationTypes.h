//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Convergence.h"
#include "ConvergenceIterationTypeEnum.h"

namespace moose
{
namespace internal
{

/**
 * Registry for Convergence iteration types.
 */
class ConvergenceIterationTypeRegistry
{
public:
  /**
   * Registers a Convergence iteration type
   * @param name The name of the Convergence iteration type
   */
  const Convergence::IterationType & registerType(const std::string & name);

  /// Return Singleton instance
  static ConvergenceIterationTypeRegistry & getRegistry();

  ///@{ Don't allow creation through copy/move construction or assignment
  ConvergenceIterationTypeRegistry(ConvergenceIterationTypeRegistry const &) = delete;
  ConvergenceIterationTypeRegistry & operator=(ConvergenceIterationTypeRegistry const &) = delete;

  ConvergenceIterationTypeRegistry(ConvergenceIterationTypeRegistry &&) = delete;
  ConvergenceIterationTypeRegistry & operator=(ConvergenceIterationTypeRegistry &&) = delete;
  ///@}

private:
  // Private constructor for singleton pattern
  ConvergenceIterationTypeRegistry() {}

  /// The registered types
  ConvergenceIterationTypeEnum _types;
};

} // internal
} // moose

namespace ConvergenceIterationTypes
{
/// Registers a Convergence iteration type to the registry
const Convergence::IterationType & registerType(const std::string & it_type);

const auto NONLINEAR = registerType("NONLINEAR");
const auto LINEAR = registerType("LINEAR");
const auto MULTISYSTEM_FIXED_POINT = registerType("MULTISYSTEM_FIXED_POINT");
const auto MULTIAPP_FIXED_POINT = registerType("MULTIAPP_FIXED_POINT");
const auto STEADY_STATE = registerType("STEADY_STATE");
}
