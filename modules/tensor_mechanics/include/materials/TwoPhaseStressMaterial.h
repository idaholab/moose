/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef TWOPHASESTRESSMATERIAL_H
#define TWOPHASESTRESSMATERIAL_H

#include "Material.h"

// Forward Declarations
class TwoPhaseStressMaterial;
class RankTwoTensor;
class ElasticityTensorR4;

template<>
InputParameters validParams<TwoPhaseStressMaterial>();

/**
 * Construct a global strain from the phase strains in a manner that is consistent
 * with the construction of the global elastic energy.
 */
class TwoPhaseStressMaterial : public Material
{
public:
  TwoPhaseStressMaterial(const std::string & name,
                         InputParameters parameters);

protected:
  virtual void computeQpProperties();

  // switching function
  MaterialProperty<Real> & _h_eta;

  // phase A material properties
  std::string _base_A;
  MaterialProperty<RankTwoTensor> & _stress_A;
  MaterialProperty<ElasticityTensorR4> & _dstress_dstrain_A;

  // phase B material properties
  std::string _base_B;
  MaterialProperty<RankTwoTensor> & _stress_B;
  MaterialProperty<ElasticityTensorR4> & _dstress_dstrain_B;

  // global material properties
  std::string _base_name;
  MaterialProperty<RankTwoTensor> & _stress;
  MaterialProperty<ElasticityTensorR4> & _dstress_dstrain;
};

#endif //TWOPHASESTRESSMATERIAL_H
