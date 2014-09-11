#ifndef LINEARELASTICPFDAMAGE_H
#define LINEARELASTICPFDAMAGE_H

#include "TensorMechanicsMaterial.h"
#include "Function.h"

/*
 * Calculates strain energy available for crack growth
 * Small strain Elastic formulation
 * Void is treated separately, scales the stiffness matrix
 * Read from file (slow) or function using bilinear interpolation (fast)
 */

class LinearElasticPFDamage : public TensorMechanicsMaterial
{
public:
  LinearElasticPFDamage(const std:: string & name, InputParameters parameters);

protected:
  virtual void computeQpStrain();
  virtual void computeQpStress();
  virtual void computeQpElasticityTensor();
  virtual void update_var(RankTwoTensor &a);
  virtual void initQpStatefulProperties();
  virtual void read_prop();
  virtual void func_prop();
  virtual void calc_num_jac(RankTwoTensor &a);

  MaterialProperty<ElasticityTensorR4> & _delasticity_tensor_dc;
  MaterialProperty<ElasticityTensorR4> & _d2elasticity_tensor_dc2;
  MaterialProperty<RankTwoTensor> & _dstress_dc;

  VariableValue & _c;

  MaterialProperty<Real> & _G0_pos;
  MaterialProperty<RankTwoTensor> & _dG0_pos_dstrain;

  MaterialProperty<Real> &_d_void;
  MaterialProperty<Real> &_d_void_old;

  bool _has_function;
  Function * _function;

  Real _kvoid;
  bool _num_jac;

  std::string _void_prop_file_name;
  unsigned int _form_type;

  MaterialProperty<Real> & _c_gauss;

};

#endif //LINEARELASTICPFDAMAGE_H
