/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COSSERATSTRESSDIVERGENCETENSORS_H
#define COSSERATSTRESSDIVERGENCETENSORS_H

#include "StressDivergenceTensors.h"

// Forward Declarations
class CosseratStressDivergenceTensors;

template <>
InputParameters validParams<CosseratStressDivergenceTensors>();

/**
 * Computes grad_i(stress_{i component})
 * This is exactly the same as StressDivergenceTensors,
 * only the Jacobian entries are correct for the Cosserat case
 */
class CosseratStressDivergenceTensors : public StressDivergenceTensors
{
public:
  CosseratStressDivergenceTensors(const InputParameters & parameters);

protected:
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Number of Cosserat rotation variables supplied by user
  const unsigned int _nrots;

  /// The MOOSE variable numbers of the Cosserat rotation variables
  std::vector<unsigned int> _wc_var;
};

#endif // COSSERATSTRESSDIVERGENCETENSORS_H
