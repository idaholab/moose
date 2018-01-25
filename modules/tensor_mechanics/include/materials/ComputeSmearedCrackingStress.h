//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTESMEAREDCRACKINGSTRESS_H
#define COMPUTESMEAREDCRACKINGSTRESS_H

#include "ColumnMajorMatrix.h"
#include "ComputeMultipleInelasticStress.h"
#include "Function.h"

class ComputeSmearedCrackingStress;

template <>
InputParameters validParams<ComputeSmearedCrackingStress>();

/**
 * ComputeSmearedCrackingStress computes the stress for a finite strain
 * material with smeared cracking
 */
class ComputeSmearedCrackingStress : public ComputeMultipleInelasticStress
{
public:
  ComputeSmearedCrackingStress(const InputParameters & parameters);

  enum CRACKING_RELEASE
  {
    CR_ABRUPT = 0,
    CR_EXPONENTIAL,
    CR_POWER,
    CR_UNKNOWN
  };

  virtual void initialSetup();

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpStress();

  void updateElasticityTensor();

  virtual void crackingStressRotation();
  virtual Real computeCrackFactor(int i,
                                  Real & sigma,
                                  Real & flag_value,
                                  const Real cracking_stress,
                                  const Real cracking_alpha,
                                  const Real youngs_modulus);

  virtual unsigned int getNumKnownCrackDirs() const;
  void computeCrackStrainAndOrientation(RealVectorValue & principal_strain);

  void applyCracksToTensor(RankTwoTensor & tensor, const RealVectorValue & sigma);

  bool previouslyCracked();

  bool _is_finite_strain;

  ///@{ Input parameters for smeared crack models
  const CRACKING_RELEASE _cracking_release;
  const Real _cracking_residual_stress;
  const VariableValue & _cracking_stress;

  std::vector<unsigned int> _active_crack_planes;
  const unsigned int _max_cracks;
  const Real _cracking_neg_fraction;

  const Real _cracking_beta;
  const Real _shear_retention_factor;
  const Real _max_stress_correction;
  ///@}

  //@{ Stateful material properties related to smeared cracking model
  MaterialProperty<RealVectorValue> & _crack_flags;
  const MaterialProperty<RealVectorValue> & _crack_flags_old;
  MaterialProperty<RealVectorValue> * _crack_count;
  const MaterialProperty<RealVectorValue> * _crack_count_old;
  MaterialProperty<RankTwoTensor> & _crack_rotation;
  const MaterialProperty<RankTwoTensor> & _crack_rotation_old;
  MaterialProperty<RealVectorValue> & _crack_strain;
  const MaterialProperty<RealVectorValue> & _crack_strain_old;
  MaterialProperty<RealVectorValue> & _crack_max_strain;
  const MaterialProperty<RealVectorValue> & _crack_max_strain_old;
  ///@}

  //@{ Variables used by multiple methods within the calculation for a single material point
  RankFourTensor _local_elasticity_tensor;
  ///@}
};

#endif // COMPUTESMEAREDCRACKINGSTRESS_H
