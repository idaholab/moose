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

#include "MFEMComplexEssentialConstraint.h"
#include "MFEMBoundaryRestrictable.h"
#include "MFEMTreeCotreeGauge.h"

/**
 * Complex (time-harmonic) counterpart of MFEMTreeCotreeGaugeEssentialConstraint.
 * The tree-cotree degrees of freedom are chosen exactly as in the real case
 * (shared via Moose::MFEM::TreeCotreeGauge); here the real and imaginary
 * components of the gauged edge dofs are both strongly set to zero.
 */
class MFEMComplexTreeCotreeGaugeEssentialConstraint : public MFEMComplexEssentialConstraint,
                                                      public MFEMBoundaryRestrictable
{
public:
  static InputParameters validParams();

  MFEMComplexTreeCotreeGaugeEssentialConstraint(const InputParameters & parameters);

  void ApplyConstraint(mfem::ParComplexGridFunction & gridfunc,
                       mfem::Array<int> & ess_tdof_list) override;

protected:
  /// Selects and caches the gauged true dofs.
  Moose::MFEM::TreeCotreeGauge _gauge;
};

#endif
