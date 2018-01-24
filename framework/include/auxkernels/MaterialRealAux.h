/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
