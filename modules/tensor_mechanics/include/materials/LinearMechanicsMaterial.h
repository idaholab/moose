// Original class author: A.M. Jokisaari,  O. Heinonen

#if 0
#ifndef LINEARMECHANICSMATERIAL_H
#define LINEARMECHANICSMATERIAL_H

#include "Material.h"
#include "RankTwoTensorTonks.h"
#include "ElasticityTensorR4.h"

//Forward declaration
class TensorMechanicsMaterial;

template<>
InputParameters validParams<TensorMechanicsMaterial>();

/**
 * TensorMechanicsMaterial handles a fully anisotropic, single-crystal material's elastic
 * constants.  It takes all 21 independent stiffness tensor inputs, or only 9, depending on the
 * boolean input value given.  This can be extended or simplified to specify HCP, monoclinic,
 * cubic, etc as needed.
 */
class TensorMechanicsMaterial : public Material
{
public:
  TensorMechanicsMaterial(const std::string & name, InputParameters parameters);

protected:
  virtual void computeQpProperties();
  virtual void computeQpElasticityTensor();
  virtual void computeQpStrain();

  virtual void computeQpStress() = 0;

  VariableGradient & _grad_disp_x;
  VariableGradient & _grad_disp_y;
  VariableGradient & _grad_disp_z;

  VariableGradient & _grad_disp_x_old;
  VariableGradient & _grad_disp_y_old;
  VariableGradient & _grad_disp_z_old;

  MaterialProperty<RankTwoTensorTonks> & _stress;
  MaterialProperty<ElasticityTensorR4> & _elasticity_tensor;
  MaterialProperty<ElasticityTensorR4> & _Jacobian_mult;
  MaterialProperty<RankTwoTensorTonks> & _elastic_strain;

  Real _euler_angle_1;
  Real _euler_angle_2;
  Real _euler_angle_3;

  // vectors to get the input values
  std::vector<Real> _Cijkl_vector;

  // bool to indicate if using 9 stiffness values or all 21
  bool _all_21;

  /// Individual material information
  ElasticityTensorR4 _Cijkl;

  // MaterialProperty<RankTwoTensorTonks> & _d_stress_dT;
  RankTwoTensorTonks _strain_increment;

  RealVectorValue _Euler_angles;
};

#endif //LINEARMECHANICSMATERIAL_H

#endif
