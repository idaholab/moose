//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "StressDivergenceTensors.h"

// Forward Declarations

/**
 * Computes grad_i(stress_{i component})
 * This is exactly the same as StressDivergenceTensors,
 * only the Jacobian entries are correct for the Cosserat case
 */
class CosseratStressDivergenceTensors : public StressDivergenceTensors
{
public:
  static InputParameters validParams();

  CosseratStressDivergenceTensors(const InputParameters & parameters);

protected:
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Number of Cosserat rotation variables supplied by user
  const unsigned int _nrots;

  /// The MOOSE variable numbers of the Cosserat rotation variables
  std::vector<unsigned int> _wc_var;
};
