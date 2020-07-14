//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TauMaterial.h"

class MatrixTau;

declareADValidParams(MatrixTau);

/**
 * Material computing $\tau$ as a diagonal matrix (when solving for the conserved
 * variables of density, momentum, and total energy) or a dense matrix (when solving
 * for any other set of variables).
 */
class MatrixTau : public TauMaterial
{
public:
  MatrixTau(const InputParameters & parameters);

protected:
  virtual void computeTau() override;

  /// matrix $\tau$
  ADMaterialProperty<DenseMatrix<Real>> & _Tau;

};
