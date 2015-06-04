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
 * ComputeRotatedElasticityTensorBase defines an elasticity tensor material object with a given base name.
 */
class ComputeRotatedElasticityTensorBase : public ComputeElasticityTensorBase
{
public:
  ComputeRotatedElasticityTensorBase(const std:: string & name, InputParameters parameters);

protected:
  RealVectorValue _Euler_angles;
};

#endif //COMPUTEROTATEDELASTICITYTENSORBASE_H
