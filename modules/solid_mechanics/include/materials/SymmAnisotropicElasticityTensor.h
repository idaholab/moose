/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SYMMANISOTROPICELASTICITYTENSOR_H
#define SYMMANISOTROPICELASTICITYTENSOR_H

#include "SymmElasticityTensor.h"

class SymmAnisotropicElasticityTensor : public SymmElasticityTensor
{
public:
  SymmAnisotropicElasticityTensor();
  SymmAnisotropicElasticityTensor(std::vector<Real> & init_list, bool all_21);
  SymmAnisotropicElasticityTensor(const SymmAnisotropicElasticityTensor & a);

  virtual ~SymmAnisotropicElasticityTensor() {}

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
   * Set the material constant c11; assumes cubic material.
   */

  void setMaterialConstantc11(const Real c11);

  /**
   * Set the material constant c22; assumes cubic material.
   */

  void setMaterialConstantc12(const Real c12);

  /**
   * Set the material constant c44; assumes cubic material.
   */

  void setMaterialConstantc44(const Real c44);

  /**
   * Perform rotation around three axes
   */

  virtual void rotate(const Real a1, const Real a2, const Real a3);

  /**
   * @return the first Euler angle
   */

  Real firstEulerAngle();

  /**
   * @return the first Euler angle
   */

  Real secondEulerAngle();

  /**
   * @return the first Euler angle
   */

  Real thirdEulerAngle();

  // Debugging functions
  void show_dt_matrix();
  void show_r_matrix();

protected:
  DenseMatrix<Real> _dmat;        // 9 x 9 Material Matrix
  DenseMatrix<Real> _qdmat;       // Rotated Material Matrix
  DenseMatrix<Real> _dt;          // 6 x 6 Material Matrix
  DenseMatrix<Real> _qdt;         // Rotated Material Matrix
  DenseMatrix<Real> _r;           // Rotational Matrix
  DenseMatrix<Real> _q;           // Q = R (dyadic) R
  DenseMatrix<Real> _qt;          // Q Transpose
  std::vector<Real> _euler_angle; // Stores Euler angeles

  DenseMatrix<Real> _trans_d6_to_d9;
  // Transformation matrix from a 6 x 6 to 9 to 9

  DenseMatrix<Real> _trans_d9_to_d6;
  // Transformation matrix from a 9 x 9 to 6 x 6

  Real _c11, _c12, _c44; // Material Constants

  void form_r_matrix();
  void initialize_material_dt_matrix();
  void initialize_anisotropic_material_dt_matrix();
  void form_rotational_q_matrix();
  void form_transformation_t_matrix();
  void form_transformed_material_dmat_matrix();
  void form_transformed_material_dt_matrix();
  void form_rotated_material_qdmat_matrix();

  /**
   * Fill in the matrix.
   */

  virtual void calculateEntries(unsigned int qp);
};

#endif // ANISOTROPICELASTICITYTENSOR_H
