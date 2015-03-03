/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTE1PHASEEIGENSTRAIN_H
#define COMPUTE1PHASEEIGENSTRAIN_H

#include "ComputeStressFreeStrainBase.h"

/**
 * Compute1PhaseEigenStrain computes an Eigenstrain that is a function of a single variable defined by a base tensor and a scalar function defined in a Derivative Material.
 */
class Compute1PhaseEigenStrain : public ComputeStressFreeStrainBase
{
public:
  Compute1PhaseEigenStrain(const std:: string & name, InputParameters parameters);

protected:
  virtual void computeQpStressFreeStrain();

  VariableValue & _v;
  VariableName _v_name;

  std::string _var_dep_name;

  const MaterialProperty<Real> & _var_dep;
  const MaterialProperty<Real> & _dvar_dep_dv;
  const MaterialProperty<Real> & _d2var_dep_dv2;

  MaterialProperty<RankTwoTensor> & _delastic_strain_dv;
  MaterialProperty<RankTwoTensor> & _d2elastic_strain_dv2;

  std::vector<Real> _eigen_base;
  RankTwoTensor _eigen_base_tensor;
};

#endif //COMPUTE1PHASEEIGENSTRAIN_H
