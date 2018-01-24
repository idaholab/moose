//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MATERIALREALTENSORVALUEAUX_H
#define MATERIALREALTENSORVALUEAUX_H

// MOOSE includes
#include "MaterialAuxBase.h"

// Forward declerations
class MaterialRealTensorValueAux;

template <>
InputParameters validParams<MaterialRealTensorValueAux>();

/**
 * AuxKernel for outputting a RealTensorValue material property component to an AuxVariable
 */
class MaterialRealTensorValueAux : public MaterialAuxBase<RealTensorValue>
{
public:
  /**
   * Class constructor
   * @param parameters The input parameters for this AuxKernel
   */
  MaterialRealTensorValueAux(const InputParameters & parameters);

protected:
  virtual Real getRealValue() override;

  /// The row index to output
  unsigned int _row;

  /// The column index to output
  unsigned int _col;
};

#endif // MATERIALREALTENSORVALUEAUX_H
