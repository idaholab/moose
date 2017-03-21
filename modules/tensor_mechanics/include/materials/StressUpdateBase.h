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

// Forward declaration
class StressUpdateBase;

template <>
InputParameters validParams<StressUpdateBase>();

/**
 * StressUpdateBase is a material that is not called by MOOSE because
 * of the compute=false flag set in the parameter list.  This class is a base class
 * for materials that use Newton iterations to perform return mapping iterations
 * to compute inelastic strains and update the stress.  All materials inheriting
 * from this class must be called by a separate material, such as ComputeReturnMappingStress.
 */
class StressUpdateBase : public Material
{
public:
  StressUpdateBase(const InputParameters & parameters);

  /// This virtual method enables inheriting materials to define an iterative
  /// procedure to calculate the stress, updating the calculated stress after
  /// each iteration is completed until convergence is achieved.  This method
  /// is called by ComputeReturnMappingStress.  All inheriting classes must
  /// overwrite this method.
  virtual void updateStress(RankTwoTensor & strain_increment,
                            RankTwoTensor & inelastic_strain_increment,
                            RankTwoTensor & stress_new) = 0;

  /// Sets the value of the global variable _qp for inheriting classes
  void setQp(unsigned int qp);

  ///@{ Retained as empty methods to avoid a warning from Material.C in framework. These methods are unused in all inheriting classes and should not be overwritten.
  void resetQpProperties() final {}
  void resetProperties() final {}
  ///@}

protected:
  const std::string _base_name;
  const MaterialProperty<RankFourTensor> & _elasticity_tensor;
  const MaterialProperty<RankTwoTensor> & _elastic_strain_old;
};

#endif // STRESSUPDATEBASE_H
