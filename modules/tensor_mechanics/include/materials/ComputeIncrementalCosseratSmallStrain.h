 /****************************************************************/
 /* MOOSE - Multiphysics Object Oriented Simulation Environment  */
 /*                                                              */
 /*          All contents are licensed under LGPL V2.1           */
 /*             See LICENSE for full restrictions                */
 /****************************************************************/
 #ifndef COMPUTEINCREMENTALCOSSERATSMALLSTRAIN_H
 #define COMPUTEINCREMENTALCOSSERATSMALLSTRAIN_H

 #include "ComputeCosseratSmallStrain.h"

 /**
  * ComputeIncrementalCosseratSmallStrain defines Cossserat strain tensor, assuming small strains.
  */
 class ComputeIncrementalCosseratSmallStrain : public ComputeCosseratSmallStrain
 {
 public:
   ComputeIncrementalCosseratSmallStrain(const InputParameters & parameters);

 protected:
   virtual void computeQpProperties();

   virtual void initQpStatefulProperties();

  /// The Cosserat rotations
  std::vector<const VariableValue *> _wc_old;

  /// Grad(Cosserat rotation)
  std::vector<const VariableGradient *> _grad_wc_old;

  MaterialProperty<RankTwoTensor> & _strain_rate;
  MaterialProperty<RankTwoTensor> & _strain_increment;
  MaterialProperty<RankTwoTensor> & _mechanical_strain_old;
  MaterialProperty<RankTwoTensor> & _total_strain_old;
  MaterialProperty<RankTwoTensor> & _rotation_increment;
  MaterialProperty<RankTwoTensor> & _deformation_gradient;

  const MaterialProperty<RankTwoTensor> & _stress_free_strain_increment;

   /// the Cosserat curvature strain: curvature_ij = nabla_j CosseratRotation_i
  MaterialProperty<RankTwoTensor> & _curvature_old;

   /// the Cosserat curvature strain: curvature_ij = nabla_j CosseratRotation_i
  MaterialProperty<RankTwoTensor> & _curvature_increment;
};

#endif //COMPUTEINCREMENTALCOSSERATSMALLSTRAIN_H
