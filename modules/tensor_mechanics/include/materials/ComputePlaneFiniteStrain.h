/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEPLANEFINITESTRAIN_H
#define COMPUTEPLANEFINITESTRAIN_H

#include "Compute2DFiniteStrain.h"

/**
 * ComputePlaneFiniteStrain defines strain increment and rotation
 * increment for finite strain under 2D planar assumptions.
 */
class ComputePlaneFiniteStrain : public Compute2DFiniteStrain
{
public:
  ComputePlaneFiniteStrain(const InputParameters & parameters);

protected:
  virtual Real computeGradDispZZ();
  virtual Real computeGradDispZZold();

  const bool _scalar_strain_zz_coupled;
  const VariableValue & _scalar_strain_zz;
  const VariableValue & _scalar_strain_zz_old;

  const bool _strain_zz_coupled;
  const VariableValue & _strain_zz;
  const VariableValue & _strain_zz_old;
};

#endif //COMPUTEPLANEFINITESTRAIN_H
