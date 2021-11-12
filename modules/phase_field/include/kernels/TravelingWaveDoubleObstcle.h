#pragma once

#include "KernelValue.h"

/**
 * TravelingWaveDoubleObstcle is the traveling wave kernel for double obstacle potentials
 * governing equation:
 *   tau * phi_dot = epsilon * laplace(phi) + gamma * (phi - 0.5) + sqrt(phi * (1 - phi)) * m;
 * coefficients:
 *   epsilon = 8 * sigma * eta / pi / pi, gamma = 8 * sigma / eta,
 *   tau = 8 * eta / mu / pi / pi,        m = -8 / pi * delta_g
 * parameters:
 *   delta_g = 1.0, eta = 6.0, mu = 1.0, sigma = 1.0
 * this kernel:
 *      -gamma * (phi - 0.5) - sqrt(phi * (1 - phi)) * m;
 * reference:
 *   I. Steinbach, Modelling and Simulation in Materials Science and Engineering. 17(7) (2009)
 *   073001. website: https://iopscience.iop.org/article/10.1088/0965-0393/17/7/073001/pdf
 * analytical solution: Eq.(67) in the reference
 */

class TravelingWaveDoubleObstcle : public KernelValue
{
public:
  static InputParameters validParams();
  TravelingWaveDoubleObstcle(const InputParameters & parameters);

protected:
  virtual Real precomputeQpResidual() override;
  virtual Real precomputeQpJacobian() override;

  /// interface energy
  const MaterialProperty<Real> & _sigma;
  /// interface width
  const MaterialProperty<Real> & _eta;
  /// driving force
  const MaterialProperty<Real> & _delta_g;
};
