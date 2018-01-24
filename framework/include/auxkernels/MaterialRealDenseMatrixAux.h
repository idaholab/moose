//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MATERIALREALDENSEMATRIXAUX_H
#define MATERIALREALDENSEMATRIXAUX_H

// MOOSE includes
#include "MaterialAuxBase.h"

// Forward declarations
class MaterialRealDenseMatrixAux;

template <>
InputParameters validParams<MaterialRealDenseMatrixAux>();

/**
 * AuxKernel for outputting a DenseMatrix<Real> material property component to an AuxVariable
 */
class MaterialRealDenseMatrixAux : public MaterialAuxBase<DenseMatrix<Real>>
{
public:
  /**
   * Class constructor
   * @param parameters The input parameters for this AuxKernel
   */
  MaterialRealDenseMatrixAux(const InputParameters & parameters);

protected:
  /// Returns the component of the tensor for output
  virtual Real getRealValue() override;

  /// The row index to output
  unsigned int _row;

  /// The column index to output
  unsigned int _col;
};

#endif // MATERIALREALDENSEMATRIXAUX_H
