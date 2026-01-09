//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

/*
Everything goes in this header for now!!

*/

#ifdef MOOSE_MFEM_ENABLED

#pragma once
#include "MFEMKernel.h"

class MFEMDGKernel : public MFEMKernel {
public:
  MFEMDGKernel(const InputParameters & parameters);
};

// all this class needs to do differently is to implement createDGBFIntegrator
class MFEMDGDiffusionKernel : public MFEMDGKernel
{
public:
  MFEMDGDiffusionKernel(const InputParameters & parameters);

  virtual mfem::BilinearFormIntegrator * createDGBFIntegrator() override;
private:
  mfem::ConstantCoefficient _one;
  mfem::ConstantCoefficient _zero;
};

// all this class needs to do differently is to implement createDGLFIntegrator
class MFEMDGDirichletLFKernel : public MFEMDGKernel
{
public:
  MFEMDGDirichletLFKernel(const InputParameters & parameters);
  virtual mfem::LinearFormIntegrator * createDGLFIntegrator() override;
private:
  mfem::ConstantCoefficient _one;
  mfem::ConstantCoefficient _zero;
};


#endif
