/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef RECOMPUTEGENERALRETURN_H
#define RECOMPUTEGENERALRETURN_H

#include "Material.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include "Conversion.h"

//Forward declaration
class RecomputeGeneralReturn;

template<>
InputParameters validParams<RecomputeGeneralReturn>();

/**
 * RecomputeGeneralReturn computes the radial return stress increment for
 * an isotropic viscoplasticity plasticity model after interating on the difference
 * between new and old trial stress increments.  This radial return mapping class
 * acts as a base class for the radial return creep and plasticity classes / combinations
 * and inherits from DiscreteMaterial.
 *
 * The stress increment computed by RecomputeGeneralReturn is used by
 * ComputeRadialReturnMappingStress which computes the elastic stress for finite
 * strains.  This return mapping class is acceptable for finite strains.
 * This class is based on the Elasto-viscoplasticity algorithm in F. Dunne and N.
 * Petrinic's Introduction to Computational Plasticity (2004) Oxford University Press.
 */

class RecomputeGeneralReturn : public Material
{
public:
  RecomputeGeneralReturn(const InputParameters & parameters);

protected:
  void computeQpProperties();

  /// In resetQpProperties, values for the recomputed material Material Properties
  /// need to be set to a constant, ideally zero, as in initQpStatefulProperties
  void resetQpProperties();

  /// The specific return mapping methods (e.g. radial/ J2, rate dependent plasticity)
  /// should be defined in inheriting material by overwritting computeStressIncrement.
  virtual void computeInelasticStrainIncrement() = 0;

  std::string _base_name;

  MaterialProperty<RankTwoTensor> & _inelastic_strain_increment;

  const unsigned int _max_its;

  const MaterialProperty<RankFourTensor> & _elasticity_tensor;
  const MaterialProperty<RankTwoTensor> & _strain_increment;
  const MaterialProperty<RankTwoTensor> & _elastic_strain_old;
};

#endif //RECOMPUTEGENERALRETURN_H
