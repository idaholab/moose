#ifndef COMPUTE1PHASEEIGENSTRAIN_H
#define COMPUTE1PHASEEIGENSTRAIN_H

#include "ComputeEigenStrainBase.h"

/**
 * Compute1PhaseEigenStrain computes an Eigenstrain that is a function of a single variable defined by a base tensor and a scalar function defined in a Derivative Material.
 */
class Compute1PhaseEigenStrain : public DerivativeMaterialInterface<ComputeEigenStrainBase>
{
public:
  Compute1PhaseEigenStrain(const std:: string & name, InputParameters parameters);

protected:
  virtual void computeQpEigenStrain();

  VariableValue & _v;
  VariableName _v_name;

  const MaterialProperty<Real> & _var_dep;
  const MaterialProperty<Real> & _dvar_dep_dv;
  const MaterialProperty<Real> & _d2var_dep_dv2;

  MaterialProperty<RankTwoTensor> & _delastic_strain_dv;
  MaterialProperty<RankTwoTensor> & _d2elastic_strain_dv2;

  std::vector<Real> _eigen_base;
  RankTwoTensor _eigen_base_tensor;
};

#endif //COMPUTE1PHASEEIGENSTRAIN_H
