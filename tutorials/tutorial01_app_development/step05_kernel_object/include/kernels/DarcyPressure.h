#pragma once

// Including the "Kernel" base class here so we can extend it
#include "Kernel.h"

/**
 * Computes the residual contribution: K / mu * grad_test * grad_u.
 */
 class DarcyPressure : public Kernel
 {
 public:
   static InputParameters validParams();

   DarcyPressure(const InputParameters & parameters);

 protected:
   /// Kernel objects must override computeQpResidual and computeQpJacobian
   virtual Real computeQpResidual() override;

   virtual Real computeQpJacobian() override;

   /// The variables which hold the value for K and mu
   const Real _permeability;
   const Real _viscosity;
 };
