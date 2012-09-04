// Original class author: A.M. Jokisaari,  O. Heinonen

#ifndef LINEARELASTICMATERIAL_H
#define LINEARELASTICMATERIAL_H

#include "Material.h"
#include "RankFourTensor.h"
#include "RankTwoTensor.h"

/**
 * LinearElasticMaterial handles a fully anisotropic, single-crystal material's elastic
 * constants.  It takes all 21 independent stiffness tensor inputs, or only 9, depending on the
 * boolean input value given.  This can be extended or simplified to specify HCP, monoclinic,
 * cubic, etc as needed.
 */

//Forward declaration
class LinearElasticMaterial;

template<>
InputParameters validParams<LinearElasticMaterial>();

class LinearElasticMaterial : public Material
{
public:
  LinearElasticMaterial(const std:: string & name, InputParameters parameters);

protected:
  virtual void computeQpProperties();

  virtual void computeQpElasticityTensor();

  virtual void computeQpStrain();

  virtual void computeQpStress();

  VariableGradient & _grad_disp_x;
  VariableGradient & _grad_disp_y;
  VariableGradient & _grad_disp_z;

  MaterialProperty<RankTwoTensor> & _stress;
  MaterialProperty<RankFourTensor> & _elasticity_tensor;
  MaterialProperty<RankFourTensor> & _Jacobian_mult;
  MaterialProperty<RankTwoTensor> & _elastic_strain;

  Real _euler_angle_1;
  Real _euler_angle_2;
  Real _euler_angle_3;
  
  // vectors to get the input values
  std::vector<Real> _Cijkl_vector;
  
  // bool to indicate if using 9 stiffness values or all 21
  bool _all_21;

  //Individual material information
  RankFourTensor _Cijkl;
  
//  MaterialProperty<RankTwoTensor> & _d_stress_dT;
  
private:

};

#endif //LINEARGENERALANISOTROPICMATERIAL_H
