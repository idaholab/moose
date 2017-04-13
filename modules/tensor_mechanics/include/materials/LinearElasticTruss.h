/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef LINEARELASTICTRUSS_H
#define LINEARELASTICTRUSS_H

#include "TrussMaterial.h"

class LinearElasticTruss : public TrussMaterial
{
public:
  LinearElasticTruss(const InputParameters & parameters);

protected:
  virtual void computeQpStrain();
  virtual void computeQpStress();

private:
  const VariableValue & _T;

  Real _T0;
  Real _thermal_expansion_coeff;
};

#endif // LINEARELASTICTRUSS_H
