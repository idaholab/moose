#include "SymmAnisotropicElasticityTensor.h"
#include <cmath>
#include <ostream>

SymmAnisotropicElasticityTensor::SymmAnisotropicElasticityTensor()
  : SymmElasticityTensor(false),
    _dmat(9,9),
    _qdmat(9,9),
    _dt(6,6),
    _qdt(6,6),
    _r(3,3),
    _q(9,9),
    _qt(9,9),
    _euler_angle(3),
    _trans_d6_to_d9(9,6),
    //_transpose_trans_d6_to_d9(6,9),
    _trans_d9_to_d6(6,9),
    //_transpose_trans_d9_to_d6(9,6),
    _c11(0),
    _c12(0),
    _c44(0),
    _cubic(false)
{
  form_transformation_t_matrix();
}

SymmAnisotropicElasticityTensor::SymmAnisotropicElasticityTensor(std::vector<Real> & init_list, bool all_21)
    : SymmElasticityTensor(false),
      _dmat(9,9),
      _qdmat(9,9),
      _dt(6,6),
      _qdt(6,6),
      _r(3,3),
      _q(9,9),
      _qt(9,9),
      _euler_angle(3),
      _trans_d6_to_d9(9,6),
      //_transpose_trans_d6_to_d9(6,9),
      _trans_d9_to_d6(6,9),
      //_transpose_trans_d9_to_d6(9,6),
      _c11(0),
      _c12(0),
      _c44(0),
      _cubic(false)
{
// test the input vector length to make sure it's correct
  if ((all_21==true && init_list.size()!=21) ||(all_21==false && init_list.size()!=9) )
    mooseError("Please correct the number of entries in the stiffness input.");
  
  if(all_21 == true)
  {
    for (int i = 0; i < 21; i++)
      _val[i] = init_list[i];
  }
  else
  {
    _val[0] = init_list[0];   //C1111
    _val[1] = init_list[1];   //C1122
    _val[2] = init_list[2];   //C1133
    _val[6] = init_list[3];   //C2222
    _val[7] = init_list[4];   //C2233
    _val[11] = init_list[5];  //C3333
    _val[15] = init_list[6];  //C2323
    _val[18] = init_list[7];  //C1313
    _val[20] = init_list[8];  //C1212
  }
  
  form_transformation_t_matrix();
}

SymmAnisotropicElasticityTensor::SymmAnisotropicElasticityTensor(const SymmAnisotropicElasticityTensor & a)
     : SymmElasticityTensor(false)
 {
   *this = a;
 }

void SymmAnisotropicElasticityTensor::setFirstEulerAngle(const Real a1)
{
  _euler_angle[0] = a1;
}

void SymmAnisotropicElasticityTensor::setSecondEulerAngle(const Real a2)
{
  _euler_angle[1] = a2;
}

void SymmAnisotropicElasticityTensor::setThirdEulerAngle(const Real a3)
{
  _euler_angle[2] = a3;
}

void SymmAnisotropicElasticityTensor::setMaterialConstantc11(const Real c11)
{
  _c11 = c11;
  if(_cubic)
  {
    _val[0] = _val[6] = _val[11] = _c11;
  }
  
}

void SymmAnisotropicElasticityTensor::setMaterialConstantc12(const Real c12)
{
  _c12 = c12;
  if(_cubic)
  {
    _val[1] = _val[2] = _val[7] = _c12;
  }
}

void SymmAnisotropicElasticityTensor::setMaterialConstantc44(const Real c44)
{
  _c44 = c44;
  if(_cubic)
  {
    _val[15] = _val[18] = _val[20] = _c44;
  }
  
}

void
SymmAnisotropicElasticityTensor::rotate(const Real a1, const Real a2, const Real a3)
{
  setFirstEulerAngle(a1);
  setSecondEulerAngle(a2);
  setThirdEulerAngle(a3);

// pulled from calculateEntries to sub in the initialize_anisotropic_material_dt_matrix() call
  form_r_matrix();
  initialize_anisotropic_material_dt_matrix();
  form_rotational_q_matrix();
  // form_transformation_t_matrix();
  form_transformed_material_dmat_matrix();
  form_rotated_material_qdmat_matrix();
  form_transformed_material_dt_matrix();

  unsigned count(0);

  for (int j(0); j < 6; ++j)
  {
    for(int i(j); i < 6; ++i)
    {
      _val[count++] = _dt(i,j);
    }
  }
}

void
SymmAnisotropicElasticityTensor::setCubic()
{
  _cubic = true;
}


void SymmAnisotropicElasticityTensor::form_r_matrix()
{
  Real phi1 = _euler_angle[0] * (M_PI/180.0);
  Real phi = _euler_angle[1] * (M_PI/180.0);
  Real phi2 = _euler_angle[2] * (M_PI/180.0);

  Real cp1 = std::cos(phi1);
  Real cp2 = std::cos(phi2);
  Real cp = std::cos(phi);

  Real sp1 = std::sin(phi1);
  Real sp2 = std::sin(phi2);
  Real sp = std::sin(phi);

  _r(0,0) = cp1 * cp2 - sp1 * sp2 * cp;
  _r(0,1) = sp1 * cp2 + cp1 * sp2 * cp;
  _r(0,2) = sp2 * sp;
  _r(1,0) = -cp1 * sp2 - sp1 * cp2 * cp;
  _r(1,1) = -sp1 * sp2 + cp1 * cp2 * cp;
  _r(1,2) = cp2 * sp;
  _r(2,0) = sp1 * sp;
  _r(2,1) = -cp1 * sp;
  _r(2,2) = cp;
}

//SMN: this function is obsolete
void
SymmAnisotropicElasticityTensor::initialize_material_dt_matrix()
{

   // This function initializes the 6 x 6 material Dt matrix

  _dt(0,0) = _dt(1,1) = _dt(2,2) = _c11;
  _dt(0,1) = _dt(0,2) = _dt(1,0) = _dt(2,0) = _dt(1,2) = _dt(2,1) = _c12;
  _dt(3,3) = _dt(4,4) = _dt(5,5) = 2 * _c44;

}

void
SymmAnisotropicElasticityTensor::initialize_anisotropic_material_dt_matrix()
{
// This function initializes the 6 x 6 material Dt matrix for an anisotropic material

  int k = 0;
  for (int i=0; i<6; i++)
  {
    for (int j=i; j<6; j++)
    {
      _dt(i,j) = _dt(j,i) = _val[k++];
    }
  }
  _dt(3,3) *= 2.0;
  _dt(4,4) *= 2.0;
  _dt(5,5) *= 2.0;
}

void
SymmAnisotropicElasticityTensor::form_rotational_q_matrix()
{

  for(int i = 0; i < 3; ++i)
    for(int j = 0; j < 3; ++j)
      for (int k = 0; k < 3; ++k)
        for(int l = 0; l < 3; ++l)
          _q(((i*3)+k),((j*3)+l)) = _r(i,j) * _r(k,l);


  /*for(int p = 0; p < 9; ++p)
    for(int q = 0; q < 9; ++q)
    _qt(q,p) = _q(p,q);*/
}

void
SymmAnisotropicElasticityTensor::form_transformation_t_matrix()
{

  // Forms to two kinds of transformation matrix
  // TransD6toD9 transfroms Dt[6][6] to Dmat[9][9]
  // TransD9toD6 transforms Dmat[9][9] to Dt[6][6]

  Real sqrt2 = std::sqrt(2.0);
  Real a = 1/sqrt2;

  _trans_d6_to_d9(0,0) = _trans_d6_to_d9(4,1) =  _trans_d6_to_d9(8,2) = 1.0;
  _trans_d6_to_d9(1,3) = _trans_d6_to_d9(3,3) = a;
  _trans_d6_to_d9(5,4) = _trans_d6_to_d9(7,4) = a;
  _trans_d6_to_d9(2,5) = _trans_d6_to_d9(6,5) = a;

  /*for(int i = 0; i < 9; ++i)
  {
    for(int j = 0; j < 6; ++j)
    {
      _transpose_trans_d6_to_d9(j,i) = _trans_d6_to_d9(i,j);
    }
    }*/

  _trans_d9_to_d6(0,0) = _trans_d9_to_d6(1,4) =  _trans_d9_to_d6(2,8) = 1.0;
  _trans_d9_to_d6(3,3) = _trans_d9_to_d6(4,7) =  _trans_d9_to_d6(5,6) = sqrt2;


  /*for(int i = 0; i < 6; ++i)
  {
    for(int j = 0; j < 9; ++j)
    {
      _transpose_trans_d9_to_d6(j,i) =  _trans_d9_to_d6(i,j);
    }
    }*/

}

void SymmAnisotropicElasticityTensor::form_transformed_material_dmat_matrix()
{

  // THIS TRANSFORMATION IS VALID ONLY WHEN THE INCOMING MATRIX HAS NOT BEEN ROTATED

  // The function makes use of TransD6toD9 matrix to transfrom
  // Dt[6][6] to Dmat[9][9]
  // Dmat = T * Dt * TT

  DenseMatrix<Real> outputMatrix(9,6);

  outputMatrix = _trans_d6_to_d9;
  outputMatrix.right_multiply(_dt);

  _dmat = outputMatrix;
  _dmat.right_multiply_transpose(_trans_d6_to_d9);

}

void SymmAnisotropicElasticityTensor::form_transformed_material_dt_matrix()
{

  // The function makes use of TransD6toD9 matrix to transfrom
  // QDmat[9][9] to Dt[6][6]
  // Dt = TT * QDmat * T
  
//   DenseMatrix<Real> outputMatrix(6,9);

//   outputMatrix = _trans_d9_to_d6;
//   outputMatrix.right_multiply(_qdmat);
//   _dt = outputMatrix;
//   _dt.right_multiply_transpose(_trans_d9_to_d6);
  
  // The transformation below is general and should work whether or not the
  //   incoming tensor has been rotated.

  ColumnMajorMatrix tmp(9,9);
  for (unsigned j(0); j < 9; ++j)
  {
    for (unsigned i(0); i < 9; ++i)
    {
      tmp(i,j) = _qdmat(i,j);
    }
  }
  
  SymmElasticityTensor fred;
  fred.convertFrom9x9( tmp );
  ColumnMajorMatrix wilma = fred.columnMajorMatrix6x6();
  for (unsigned j(0); j < 6; ++j)
  {
    for (unsigned i(0); i < 6; ++i)
    {
      _dt(i,j) = wilma(i,j);
    }
  }
}


void
SymmAnisotropicElasticityTensor::form_rotated_material_qdmat_matrix()
{

  // The function makes use of Q matrix to rotate
  // Dmat[9][9] to QDmat[9][9]
  // QDmat = QT * Dmat * Q

  DenseMatrix<Real> outputMatrix(9,9);

  _q.get_transpose(outputMatrix);
  outputMatrix.right_multiply(_dmat);

  _qdmat = outputMatrix;
  _qdmat.right_multiply(_q);

}

void
SymmAnisotropicElasticityTensor::calculateEntries(unsigned int /*qp*/)
{

  form_r_matrix();
  initialize_anisotropic_material_dt_matrix();
  form_rotational_q_matrix();
  // form_transformation_t_matrix();
  form_transformed_material_dmat_matrix();
  form_rotated_material_qdmat_matrix();
  form_transformed_material_dt_matrix();

  unsigned count(0);

  for (int j(0); j < 6; ++j)
  {
    for(int i(j); i < 6; ++i)
    {
      _val[count++] = _dt(i,j);
    }
  }

}
