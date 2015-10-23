/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEELASTICSMEAREDCRACKINGSTRESS_H
#define COMPUTEELASTICSMEAREDCRACKINGSTRESS_H

#include "ComputeStressBase.h"
#include "Function.h"

/**
 * ComputeElasticSmearedCrackingStress computes the stress for a finite strain elastic
 * model with smeared cracking
 */
class ComputeElasticSmearedCrackingStress : public ComputeStressBase
{
public:
  ComputeElasticSmearedCrackingStress(const InputParameters & parameters);

  enum CRACKING_RELEASE
  {
    CR_ABRUPT = 0,
    CR_EXPONENTIAL,
    CR_POWER,
    CR_UNKNOWN
  };

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();
  virtual void computeQpStress();

  void updateElasticityTensor();

  virtual void crackingStressRotation();
  virtual Real computeCrackFactor(int i, Real & sigma, Real & flagVal);

  virtual unsigned int getNumKnownCrackDirs() const;
  void computeCrackStrainAndOrientation(ColumnMajorMatrix & principal_strain);

  void applyCracksToTensor(RankTwoTensor & tensor, const RealVectorValue & sigma);

  const MaterialProperty<RankTwoTensor> & _mechanical_strain;

  const bool _is_finite_strain;

  //materail properties related to stress and strain
  const MaterialProperty<RankTwoTensor> & _strain_increment;
  const MaterialProperty<RankTwoTensor> & _rotation_increment;
  MaterialProperty<RankTwoTensor> & _stress_old;
  MaterialProperty<RankTwoTensor> & _elastic_strain_old;

  //input parameters for smeared crack models
  const CRACKING_RELEASE _cracking_release;
  Real _cracking_stress;
  const Real _cracking_residual_stress;
  Function * const _cracking_stress_function;

  Real _cracking_alpha;
  std::vector<unsigned int> _active_crack_planes;
  const unsigned int _max_cracks;
  const Real _cracking_neg_fraction;
  RealVectorValue _crack_flags_local;
  ColumnMajorMatrix _principal_strain;

  //stateful material properties related to smeared cracking model
  MaterialProperty<RealVectorValue> * _crack_flags;
  MaterialProperty<RealVectorValue> * _crack_flags_old;
  MaterialProperty<RealVectorValue> * _crack_count;
  MaterialProperty<RealVectorValue> * _crack_count_old;
  MaterialProperty<RankTwoTensor> * _crack_rotation;
  MaterialProperty<RankTwoTensor> * _crack_rotation_old;
  MaterialProperty<RealVectorValue> * _crack_strain;
  MaterialProperty<RealVectorValue> * _crack_strain_old;
  MaterialProperty<RealVectorValue> * _crack_max_strain;
  MaterialProperty<RealVectorValue> * _crack_max_strain_old;

  RankFourTensor  _local_elasticity_tensor;

  Real _youngs_modulus;

  template<typename T>
  MaterialProperty<T> & createProperty(const std::string & prop_name)
  {
    std::string name(_base_name + prop_name);
    return declareProperty<T>(name);
  }

  template<typename T>
  MaterialProperty<T> & createPropertyOld(const std::string & prop_name)
  {
    std::string name(_base_name + prop_name);
    return declarePropertyOld<T>(name);
  }
};

#endif // COMPUTEELASTICSMEAREDCRACKINGSTRESS_H
