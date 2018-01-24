/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEROTATEDELASTICITYTENSORBASE_H
#define COMPUTEROTATEDELASTICITYTENSORBASE_H

#include "ComputeElasticityTensorBase.h"

/**
 * ComputeRotatedElasticityTensorBase is an intermediate base class that rotates an elasticity
 * tensor based on euler angles.
 */
class ComputeRotatedElasticityTensorBase : public ComputeElasticityTensorBase
{
public:
  ComputeRotatedElasticityTensorBase(const InputParameters & parameters);

protected:
  RealVectorValue _Euler_angles;
};

#endif // COMPUTEROTATEDELASTICITYTENSORBASE_H
