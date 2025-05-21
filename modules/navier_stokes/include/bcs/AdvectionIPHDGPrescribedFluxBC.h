//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IPHDGPrescribedFluxBC.h"

/**
 * Implements a prescribed advective flux boundary condition for use with a hybridized
 * discretization of an advection term. This BC may most often be used to prescribe zero advective
 * flux conditions at wall-type boundaries but it could also be used to prescribe inlet advective
 * fluxes
 */
class AdvectionIPHDGPrescribedFluxBC : public IPHDGPrescribedFluxBC
{
public:
  static InputParameters validParams();
  AdvectionIPHDGPrescribedFluxBC(const InputParameters & parameters);

protected:
  virtual IPHDGAssemblyHelper & iphdgHelper() override { return *_iphdg_helper; }

  /// The assembly helper providing the required IP-HDG method implementations
  std::unique_ptr<AdvectionIPHDGAssemblyHelper> _iphdg_helper;
};
