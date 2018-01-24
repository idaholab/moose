//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MATERIALREALVECTORVALUEAUX_H
#define MATERIALREALVECTORVALUEAUX_H

// MOOSE includes
#include "MaterialAuxBase.h"

// Forward declarations
class MaterialRealVectorValueAux;

template <>
InputParameters validParams<MaterialRealVectorValueAux>();

/**
 * AuxKernel for outputting a RealVectorValue material property component to an AuxVariable
 */
class MaterialRealVectorValueAux : public MaterialAuxBase<RealVectorValue>
{
public:
  /**
   * Class constructor
   * @param parameters The input parameters for this object
   */
  MaterialRealVectorValueAux(const InputParameters & parameters);

protected:
  virtual Real getRealValue() override;

  /// The vector component to output
  unsigned int _component;
};

#endif // MATERIALREALVECTORVALUEAUX_H
