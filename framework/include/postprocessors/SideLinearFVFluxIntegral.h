//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideIntegralPostprocessor.h"
#include <map>
#include <string>

class LinearFVBoundaryCondition;
class LinearFVFluxKernel;

/**
 * Computes the side integral of fluxes from selected LinearFVFluxKernel objects.
 */
class SideLinearFVFluxIntegral : public SideIntegralPostprocessor
{
public:
  static InputParameters validParams();
  virtual void initialSetup() override;

  SideLinearFVFluxIntegral(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;
  virtual Real computeFaceInfoIntegral(const FaceInfo * fi) override;

  /// Names of the kernels whose boundary flux we want to integrate
  const std::vector<std::string> & _kernel_names;

  /// Kernel objects to integrate
  std::vector<LinearFVFluxKernel *> _kernel_objects;

  /// Cached variable name for all kernels (must be the same)
  std::string _variable_name;

  /// Cached variable number for all kernels (must be the same)
  unsigned int _variable_number;

  /// Cached system number for all kernels (must be the same)
  unsigned int _system_number;

  /// Cached BC pointers on requested boundaries
  std::unordered_map<BoundaryID, LinearFVBoundaryCondition *> _boundary_bcs;
};
