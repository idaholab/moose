/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ACGRGRPOLY_H
#define ACGRGRPOLY_H

#include "ACGrGrBase.h"

// Forward Declarations
class ACGrGrPoly;

template <>
InputParameters validParams<ACGrGrPoly>();

/**
 * This kernel calculates the residual for grain growth for a single phase,
 * poly-crystal system. A single material property gamma_asymm is used for
 * the prefactor of the cross-terms between order parameters.
 */
class ACGrGrPoly : public ACGrGrBase
{
public:
  ACGrGrPoly(const InputParameters & parameters);

protected:
  virtual Real computeDFDOP(PFFunctionType type);
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const MaterialProperty<Real> & _gamma;
};

#endif // ACGRGRPOLY_H
