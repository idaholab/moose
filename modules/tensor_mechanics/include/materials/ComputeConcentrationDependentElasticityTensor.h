/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTECONCENTRATIONDEPENDENTELASTICITYTENSOR_H
#define COMPUTECONCENTRATIONDEPENDENTELASTICITYTENSOR_H

#include "ComputeElasticityTensor.h"

/**
 * ComputeElasticityTensor defines an elasticity tensor material object with a given base name.
 */
class ComputeConcentrationDependentElasticityTensor : public ComputeElasticityTensor
{
public:
  ComputeConcentrationDependentElasticityTensor(const std:: string & name, InputParameters parameters);

protected:
  virtual void computeQpElasticityTensor();

  // Individual material information
  ElasticityTensorR4 _Cijkl0;
  ElasticityTensorR4 _Cijkl1;
  VariableValue & _c;
  VariableName _c_name;

  MaterialProperty<ElasticityTensorR4> & _delasticity_tensor_dc;

};

#endif //COMPUTEELASTICITYTENSOR_H
