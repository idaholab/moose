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

// all this class needs to do differently is to implement createDGBFIntegrator
class MFEMDGDiffusionKernel : public MFEMDGKernel
{
public:
  MFEMDGDiffusionKernel(const InputParameters & parameters);

  virtual mfem::BilinearFormIntegrator * createDGBFIntegrator() override;
};

#endif
