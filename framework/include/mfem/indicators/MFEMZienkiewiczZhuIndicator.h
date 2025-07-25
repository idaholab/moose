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

class MFEMZienkiewiczZhuIndicator : public MFEMIndicator
{
public:
  MFEMZienkiewiczZhuIndicator(const InputParameters & parameters);
  virtual ~MFEMZienkiewiczZhuIndicator() = default;

  static InputParameters validParams();

  virtual bool createEstimator() override;

protected:
  std::unique_ptr<mfem::H1_FECollection> _smooth_flux_fec;
  std::unique_ptr<mfem::L2_FECollection> _flux_fec;
  std::unique_ptr<mfem::ParFiniteElementSpace> _smooth_flux_fes;
  std::unique_ptr<mfem::ParFiniteElementSpace> _flux_fes;
};

#endif
