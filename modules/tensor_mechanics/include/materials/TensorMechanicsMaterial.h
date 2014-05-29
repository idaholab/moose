// Original class author: A.M. Jokisaari,  O. Heinonen, M.R. Tonks

#ifndef TENSORMECHANICSMATERIAL_H
#define TENSORMECHANICSMATERIAL_H

#include "Material.h"
#include "RankTwoTensor.h"
#include "ElasticityTensorR4.h"
#include "RotationTensor.h"

/**
 * TensorMechanicsMaterial handles a fully anisotropic, single-crystal material's elastic
 * constants.  It takes all 21 independent stiffness tensor inputs, or only 9, depending on the
 * boolean input value given.  This can be extended or simplified to specify HCP, monoclinic,
 * cubic, etc as needed.
 */

//Forward declaration
class TensorMechanicsMaterial;

template<>
InputParameters validParams<TensorMechanicsMaterial>();

class TensorMechanicsMaterial : public Material
{
public:
  TensorMechanicsMaterial(const std:: string & name, InputParameters parameters);

protected:
  virtual void computeProperties();

  virtual void computeQpElasticityTensor();

  virtual void computeStrain();

  virtual void computeQpStrain() = 0;

  virtual void computeQpStress() = 0;

  VariableGradient & _grad_disp_x;
  VariableGradient & _grad_disp_y;
  VariableGradient & _grad_disp_z;

  VariableGradient & _grad_disp_x_old;
  VariableGradient & _grad_disp_y_old;
  VariableGradient & _grad_disp_z_old;

  MaterialProperty<RankTwoTensor> & _stress;
  MaterialProperty<RankTwoTensor> & _elastic_strain;
  MaterialProperty<ElasticityTensorR4> & _elasticity_tensor;
  MaterialProperty<ElasticityTensorR4> & _Jacobian_mult;

  Real _euler_angle_1;
  Real _euler_angle_2;
  Real _euler_angle_3;

  // vectors to get the input values
  std::vector<Real> _Cijkl_vector;


  //Individual material information
  ElasticityTensorR4 _Cijkl;

//  MaterialProperty<RankTwoTensor> & _d_stress_dT;
  RankTwoTensor _strain_increment;

  RealVectorValue _Euler_angles;

// Current deformation gradient
  RankTwoTensor _dfgrd;

  bool _has_T;
  VariableValue * _T; //pointer rather than reference

  /// determines the translation from C_ijkl to the Rank-4 tensor
  MooseEnum _fill_method;

  /// bool to indicate if using 9 stiffness values or all 21
  bool _all_21;


private:

};

#endif //TENSORMECHANICSMATERIAL_H
