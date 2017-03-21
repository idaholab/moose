/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef DYNAMICSTRESSDIVERGENCETENSORS_H
#define DYNAMICSTRESSDIVERGENCETENSORS_H

#include "StressDivergenceTensors.h"

/**
* DynamicStressDivergenceTensors derives from StressDivergenceTensors and adds stress related
* Rayleigh and HHT time integration terms.
*/
class DynamicStressDivergenceTensors : public StressDivergenceTensors
{
public:
  DynamicStressDivergenceTensors(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const MaterialProperty<RankTwoTensor> & _stress_older;
  const MaterialProperty<RankTwoTensor> & _stress_old;

  // Rayleigh damping parameter _zeta and HHT time integration parameter _alpha
  const MaterialProperty<Real> & _zeta;
  const Real _alpha;
  const bool _static_initialization;
};

#endif // DYNAMICSTRESSDIVERGENCETENSORS_H
