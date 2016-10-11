/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef STRESSUPDATEBASE_H
#define STRESSUPDATEBASE_H

#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include "Conversion.h"

//Forward declaration
class StressUpdateBase;

template<>
InputParameters validParams<StressUpdateBase>();

/**
 * StressUpdateBase is a material that is not called by MOOSE because
 * of the compute=false flag set in the parameter list.  This class is a base class
 * for materials that use Newton iterations to perform radial return mapping
 * iterations to compute inelastic strains.  All materials inheriting from this
 * class must be called by a separate material, such as ComputeReturnMappingStress.
 */
class StressUpdateBase : public Material
{
public:
  StressUpdateBase(const InputParameters & parameters);

  /// A radial return (J2) mapping method is defined in this material by overwritting
  /// the computeInelasticStrainIncrement method.
  virtual void updateStress(RankTwoTensor & strain_increment,
                            RankTwoTensor & inelastic_strain_increment,
                            RankTwoTensor & stress_new);

  void setQp(unsigned qp);

protected:
  const std::string _base_name;
  const MaterialProperty<RankFourTensor> & _elasticity_tensor;
  const MaterialProperty<RankTwoTensor> & _elastic_strain_old;
};

#endif //STRESSUPDATEBASE_H
