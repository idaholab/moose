//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MATERIALREALAUX_H
#define MATERIALREALAUX_H

// MOOSE includes
#include "MaterialAuxBase.h"

// Forward Declarations
class MaterialRealAux;

template <>
InputParameters validParams<MaterialRealAux>();

/**
 * Object for passing a scalar, REAL material property to an AuxVariable
 */
class MaterialRealAux : public MaterialAuxBase<Real>
{
public:
  /**
   * Class constructor.
   * @param parameters Input parameters for this object
   */
  MaterialRealAux(const InputParameters & parameters);

protected:
  /// Returns the material property values at quadrature points
  virtual Real getRealValue();
};

#endif // MATERIALREALAUX_H
