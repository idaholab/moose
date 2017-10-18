/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEEIGENSTRAINFROMINITIALSTRESS_H
#define COMPUTEEIGENSTRAINFROMINITIALSTRESS_H

#include "ComputeEigenstrainBase.h"
#include "RankFourTensor.h"

/**
 * ComputeEigenstrain computes an Eigenstrain that results from an initial stress
 */
class ComputeEigenstrainFromInitialStress : public ComputeEigenstrainBase
{
public:
  ComputeEigenstrainFromInitialStress(const InputParameters & parameters);

protected:
  virtual void computeQpEigenstrain() override;

  /// base_name for elasticity tensor to use to convert stress to strain
  const std::string _base_name;

  /// elasticity tensor used to convert stress to strain
  const MaterialProperty<RankFourTensor> & _elasticity_tensor;

  /// initial stress components
  std::vector<Function *> _initial_stress_fcn;
};

#endif // COMPUTEEIGENSTRAINFROMINITIALSTRESS_H
