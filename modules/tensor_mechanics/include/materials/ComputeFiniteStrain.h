/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEFINITESTRAIN_H
#define COMPUTEFINITESTRAIN_H

#include "ComputeStrainBase.h"

/**
 * ComputeFiniteStrain defines a strain increment and rotation increment, for finite strains.
 */
class ComputeFiniteStrain : public ComputeStrainBase
{
public:
  ComputeFiniteStrain(const InputParameters & parameters);

  virtual void computeProperties();

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpStrain();
  virtual void computeQpIncrements(RankTwoTensor & e, RankTwoTensor & r);

  MaterialProperty<RankTwoTensor> & _strain_rate;
  MaterialProperty<RankTwoTensor> & _strain_increment;
  MaterialProperty<RankTwoTensor> & _mechanical_strain_old;
  MaterialProperty<RankTwoTensor> & _total_strain_old;
  MaterialProperty<RankTwoTensor> & _rotation_increment;

  MaterialProperty<RankTwoTensor> & _deformation_gradient;
  MaterialProperty<RankTwoTensor> & _deformation_gradient_old;

  const MaterialProperty<RankTwoTensor> & _stress_free_strain_increment;

  const Real & _current_elem_volume;
  std::vector<RankTwoTensor> _Fhat;

  MooseEnum _decomposition_method;

private:
  /// True if this is the first timestep (timestep < 2). At the first
  /// timestep, the change in temperature should be calculated with the reference
  /// stress free temperature, not the stateful _temperature_old; this boolean variable
  /// eliminates the use of the _app.isRestarting() in the soon to be deprecicated
  /// calculation of thermal expansion strain in this class.
  /// This boolean is delcared as a reference so that the variable is restartable
  /// data:  if we restart, the code will not think it is the first timestep again.
  bool & _step_one;
};

#endif //COMPUTEFINITESTRAIN_H
