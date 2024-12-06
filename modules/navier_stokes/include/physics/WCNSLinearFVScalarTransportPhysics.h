//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "WCNSFVScalarTransportPhysicsBase.h"

/**
 * Creates all the objects needed to solve the Navier Stokes scalar transport equations
 * using the linear finite volume weakly-compressible discretization (WCNSFV)
 */
class WCNSLinearFVScalarTransportPhysics : public WCNSFVScalarTransportPhysicsBase
{
public:
  static InputParameters validParams();

  WCNSLinearFVScalarTransportPhysics(const InputParameters & parameters);

private:
  virtual void addSolverVariables() override;

  /**
   * Functions adding kernels for the incompressible / weakly-compressible scalar transport
   * equation
   * If the material properties are not constant, some of these can be used for
   * weakly-compressible simulations as well.
   */
  virtual void addScalarTimeKernels() override;
  virtual void addScalarDiffusionKernels() override;
  virtual void addScalarAdvectionKernels() override;
  virtual void addScalarSourceKernels() override;

  /// Functions adding boundary conditions for the incompressible simulation.
  /// These are used for weakly-compressible simulations as well.
  virtual void addScalarInletBC() override;
  virtual void addScalarWallBC() override{};
  virtual void addScalarOutletBC() override;
};
