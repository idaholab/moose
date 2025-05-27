//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IPHDGPrescribedFluxBC.h"

/**
 * Implements a fixed normal gradient boundary condition for use with a hybridized discretization of
 * the diffusion equation
 */
class DiffusionIPHDGPrescribedFluxBC : public IPHDGPrescribedFluxBC
{
public:
  static InputParameters validParams();
  DiffusionIPHDGPrescribedFluxBC(const InputParameters & parameters);

protected:
  virtual IPHDGAssemblyHelper & iphdgHelper() override { return *_iphdg_helper; }

  /// The assembly helper providing the required IP-HDG method implementations
  std::unique_ptr<DiffusionIPHDGAssemblyHelper> _iphdg_helper;
};
