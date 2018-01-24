/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ACBulk.h"

#ifndef ACGBPOLY_H
#define ACGBPOLY_H

// Forward Declarations
class ACGBPoly;

template <>
InputParameters validParams<ACGBPoly>();

class ACGBPoly : public ACBulk<Real>
{
public:
  ACGBPoly(const InputParameters & parameters);

protected:
  virtual Real computeDFDOP(PFFunctionType type);
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const VariableValue & _c;
  unsigned int _c_var;

  const MaterialProperty<Real> & _mu;
  const MaterialProperty<Real> & _gamma;

  Real _en_ratio;
};

#endif // ACGBPOLY_H
