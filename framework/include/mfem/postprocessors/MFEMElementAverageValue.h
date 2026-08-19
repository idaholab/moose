//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "MFEMPostprocessor.h"
#include "MFEMBlockRestrictable.h"

/**
 * Computes the volumetric average of a scalar MFEM variable over the mesh or
 * a subset of subdomains.
 */
class MFEMElementAverageValue : public MFEMPostprocessor, public MFEMBlockRestrictable
{
public:
  static InputParameters validParams();

  MFEMElementAverageValue(const InputParameters & parameters);

  void initialize() override {}
  void execute() override;

  PostprocessorValue getValue() const override final;

private:
  /// Reference to the MFEM grid function whose average is being computed
  mfem::ParGridFunction & _var;
  /// Linear form used to integrate the variable over elements
  mfem::ParLinearForm _lf;
  /// Volume of the domain (or restricted subdomains), computed once at construction
  const mfem::real_t _volume;
  /// Cached computed average value returned by getValue()
  mfem::real_t _value{0};
};

#endif
