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

#include "MFEMKernel.h"

/**
 * \f[
 * \langle \dot W, v \rangle
 * \f]
 */
class MFEMWhiteGaussianNoiseDomainLFKernel : public MFEMKernel
{
public:
  static InputParameters validParams();

  MFEMWhiteGaussianNoiseDomainLFKernel(const InputParameters & parameters);

  virtual mfem::LinearFormIntegrator * createLFIntegrator() override;

protected:
  /// Communicator of the variable's finite element space, used to offset the seed on each rank
  MPI_Comm _comm;
  const unsigned int _seed;
};

#endif
