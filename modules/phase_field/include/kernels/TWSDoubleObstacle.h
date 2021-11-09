#pragma once

#include "ADKernelValue.h"

/**
 * TWSDoubleObstacle is the traveling wave kernel for double obstacle potentials
 * governing equation:
 *   tao * phi_dot = epsilon * laplace(phi) + gamma * (phi - 0.5) + sqrt(phi * (1 - phi)) * m;
 * coefficients:
 *   epsilon = 8 * sigma * eta / pi / pi, gamma = 8 * sigma / eta,
 *   tao = 8 * eta / mu / pi / pi,        m = -8 / pi * delta_g
 * parameters:
 *   delta_g = 1.0, eta = 6.0, mu = 1.0, sigma = 1.0
 * this kernel:
 *      -gamma * (phi - 0.5) - sqrt(phi * (1 - phi)) * m;
 * reference:
 *   I. Steinbach, Modelling and Simulation in Materials Science and Engineering. 17(7) (2009) 073001.
 *   website: https://iopscience.iop.org/article/10.1088/0965-0393/17/7/073001/pdf 
 * analytical solution:
 *   Eq.(67) in the reference
 */

class TWSDoubleObstacle : public ADKernelValue
{
public:
  static InputParameters validParams();
  TWSDoubleObstacle(const InputParameters & parameters);

protected:
  virtual ADReal precomputeQpResidual() override;
  const std::string & _sigma_name;
  const ADMaterialProperty<Real> & _sigma;

  const std::string & _eta_name;
  const ADMaterialProperty<Real> & _eta;

  const std::string & _delta_g_name;
  const ADMaterialProperty<Real> & _delta_g;

  const double _pi = 3.14159265358979323846;
};