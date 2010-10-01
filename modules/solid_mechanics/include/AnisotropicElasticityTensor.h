#ifndef ANISOTROPICELASTICITYTENSOR_H
#define ANISOTROPICELASTICITYTENSOR_H

#include "ElasticityTensor.h"
#include "libmesh.h"
#include "vector_value.h"
#include "dense_matrix.h"
#include "mesh.h"

/**
 * Defines an Isotropic Elasticity Tensor.
 *
 * The input is any two of the following:
 *
 * Youngs Modulus
 * Poissons Ration
 * First and Second Lame Coefficients (lambda and mu)
 * Bulk Modulus
 *
 * Internally this class uses the Lame Coefficients.
 * Everything is is transformed to these coefficients.
 *
 * Note that by default this tensor is _constant_... meaning
 * that it never changes after the first time it is computed.
 *
 * If you want to modify this behavior you can pass in
 * false to the constructor.
 */
class AnisotropicElasticityTensor : public ElasticityTensor
{
public:
  AnisotropicElasticityTensor(const bool constant = true);
  
  /**
   * Set the first euler angle
   */
  
  void setFirstEulerAngle(const Real a1);

  /**
   * Set the second euler angle
   */
  
  void setSecondEulerAngle(const Real a2);

  /**
   * Set the third euler angle
   */
  
  void setThirdEulerAngle(const Real a3);

  /**
   * Set the material constant c11
   */
  
  void setMaterialConstantc11(const Real c11);

  /**
   * Set the material constant c22
   */
  
  void setMaterialConstantc12(const Real c12);

  /**
   * Set the material constant c44
   */
  
  void setMaterialConstantc44(const Real c44);
  
  
protected:
  
  DenseMatrix<Real> _dmat; // 9 x 9 Material Matrix
  DenseMatrix<Real> _qdmat; // Rotated Material Matrix
  DenseMatrix<Real> _dt; // 6 x 6 Material Matrix
  DenseMatrix<Real> _qdt; // Rotated Material Matrix
  DenseMatrix<Real> _r; // Rotational Matrix
  DenseMatrix<Real> _q; // Q = R (dyadic) R
  DenseMatrix<Real> _qt; // Q Transpose
  std::vector<Real> _euler_angle; // Stores Euler angeles
  
  DenseMatrix<Real> _trans_d6_to_d9, _transpose_trans_d6_to_d9;
  // Transformation matrix from a 6 x 6 to 9 to 9
  
  DenseMatrix<Real> _trans_d9_to_d6, _transpose_trans_d9_to_d6;
  // Transformation matrix from a 9 x 9 to 6 x 6
  
  Real _c11, _c12, _c44; // Material Constants
  
  void form_r_matrix();
  void intialize_material_dt_matrix();
  void form_rotational_q_matrix();
  void form_transformation_t_matrix();
  void form_transformed_material_dmat_matrix(); 
  void form_rotated_material_qdmat_matrix();
  
  /**
   * Fill in the matrix.
   */

  virtual void calculateEntries();

};

#endif //ANISOTROPICELASTICITYTENSOR_H
