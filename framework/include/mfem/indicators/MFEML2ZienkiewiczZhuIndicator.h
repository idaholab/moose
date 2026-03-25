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

#include "MFEMIndicator.h"

/**
 * Wrapper for the Zienkiewicz-Zhu estimator with L2 projection.
 * See mfem example 6p for more details.
 */
class MFEML2ZienkiewiczZhuIndicator : public MFEMIndicator
{
public:
  static InputParameters validParams();

  MFEML2ZienkiewiczZhuIndicator(const InputParameters & parameters);

  /// Override the createEstimator method to use a Zienkiewicz-Zhu estimator.
  virtual void createEstimator() override;

protected:
  /// Finite element space for the smoothed flux.
  std::shared_ptr<mfem::ParFiniteElementSpace> _smooth_flux_fes;
  /// Finite element collection for the smoothed flux.
  /// Only used if the user doesn't supply an existing FESpace for the smooth flux.
  std::shared_ptr<mfem::FiniteElementCollection> _smooth_flux_fec;

  /// Finite element space for the discontinuous flux.
  std::shared_ptr<mfem::ParFiniteElementSpace> _flux_fes;
  /// Finite element collection for the discontinuous flux.
  /// Only used if the user doesn't supply an existing FESpace for the discontinuous flux.
  std::shared_ptr<mfem::FiniteElementCollection> _flux_fec;

  /// Auxiliary blf integrator implementing ComputeElementFlux().
  std::unique_ptr<mfem::BilinearFormIntegrator> _integ;
};

#endif
