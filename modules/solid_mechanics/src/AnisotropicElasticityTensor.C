#include "AnisotropicElasticityTensor.h"

AnisotropicElasticityTensor::AnisotropicElasticityTensor()
  : ElasticityTensor(false),
    _dmat(9,9),
    _qdmat(9,9),
    _dt(6,6),
    _qdt(6,6),
    _r(3,3),
    _q(9,9),
    _qt(9,9),
    _euler_angle(3),
    _trans_d6_to_d9(9,6),
    _transpose_trans_d6_to_d9(6,9),
    _trans_d9_to_d6(6,9),
    _transpose_trans_d9_to_d6(9,6),
    _c11(0),
    _c12(0),
    _c44(0)
   {}


void AnisotropicElasticityTensor::setFirstEulerAngle(const Real a1)
{
  _euler_angle[0] = a1;
}

void AnisotropicElasticityTensor::setSecondEulerAngle(const Real a2)
{
  _euler_angle[1] = a2;
}

void AnisotropicElasticityTensor::setThirdEulerAngle(const Real a3)
{
  _euler_angle[2] = a3;
}

void AnisotropicElasticityTensor::setMaterialConstantc11(const Real c11)
{
  _c11 = c11;
}

void AnisotropicElasticityTensor::setMaterialConstantc12(const Real c12)
{
  _c12 = c12;
}

void AnisotropicElasticityTensor::setMaterialConstantc44(const Real c44)
{
  _c44 = c44;
}

void AnisotropicElasticityTensor::form_r_matrix()
{
  double phi1, phi, phi2;
  double cp, cp1, cp2, sp, sp1, sp2;
  double const pi = 3.14159265358979;

  phi1 = _euler_angle[0] * (pi/180.0);
  phi = _euler_angle[1] * (pi/180.0);
  phi2 = _euler_angle[2] * (pi/180.0);

  cp1 = cos(phi1);
  cp2 = cos(phi2);
  cp = cos(phi);

  sp1 = sin(phi1);
  sp2 = sin(phi2);
  sp = sin(phi);

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


void AnisotropicElasticityTensor::intialize_material_dt_matrix()
{

   // This function intializes the 6 x 6 material Dt matrix

  _dt(0,0) = _dt(1,1) = _dt(2,2) = _c11;
  _dt(0,1) = _dt(0,2) = _dt(1,0) = _dt(2,0) = _dt(1,2) = _dt(2,1) = _c12;
  _dt(3,3) = _dt(4,4) = _dt(5,5) = 2 * _c44;
  
}



void AnisotropicElasticityTensor::form_rotational_q_matrix()
{
  
  for(int i = 0; i < 3; i++)
    for(int j = 0; j < 3; j++)
    {
      for (int k = 0; k < 3; k++)
        for(int l = 0; l < 3; l++)
          _q(((i*3)+k),((j*3)+l)) = _r(i,j) * _r(k,l);
    }
    
    
   for(int p = 0; p < 9; p++)
    for(int q = 0; q < 9; q++)
      _qt(q,p) = _q(p,q);
  
}


void AnisotropicElasticityTensor::form_transformation_t_matrix()
{
  
  // Forms to two kinds of transformation matrix
  // TransD6toD9 transfroms Dt[6][6] to Dmat[9][9]
  // TransD9toD6 transforms Dmat[9][9] to Dt[6][6]
  
  const double a = pow(0.5,0.5);
  
  _trans_d6_to_d9(0,0) = _trans_d6_to_d9(4,1) =  _trans_d6_to_d9(8,2) = 1.0;
  _trans_d6_to_d9(1,3) = _trans_d6_to_d9(3,3) = a;
  _trans_d6_to_d9(2,4) = _trans_d6_to_d9(6,4) = a;
  _trans_d6_to_d9(5,5) = _trans_d6_to_d9(7,5) = a;
  
  for(int i = 0; i < 9; i++)
    for(int j = 0; j < 6; j++)
      _transpose_trans_d6_to_d9(j,i) = _trans_d6_to_d9(i,j);
  
  
  const double b = pow(2.0,0.5);

   _trans_d9_to_d6(0,0) = _trans_d9_to_d6(1,4) =  _trans_d9_to_d6(2,8) = 1.0;
   _trans_d9_to_d6(3,3) = _trans_d9_to_d6(4,6) =  _trans_d9_to_d6(5,7) = b;

   
   for(int i = 0; i < 6; i++)
     for(int j = 0; j < 9; j++)
       _transpose_trans_d9_to_d6(j,i) =  _trans_d9_to_d6(i,j);

}



void AnisotropicElasticityTensor::form_transformed_material_dmat_matrix()
{

  // The function makes use of TransD6toD9 matrix to transfrom
  // Dt[6][6] to Dmat[9][9]
  // Dmat = T * Dt * TT

  DenseMatrix<Real> outputMatrix(9,6);

  outputMatrix = _trans_d6_to_d9;
  outputMatrix.right_multiply(_dt);

  _dmat = outputMatrix;
  _dmat.right_multiply(_transpose_trans_d6_to_d9);
  
}

    
void AnisotropicElasticityTensor::form_rotated_material_qdmat_matrix()
{
  
  // The function makes use of Q matrix to rotate
  // Dmat[9][9] to QDmat[9][9]
  // QDmat = QT * Dmat * Q
  
  DenseMatrix<Real> outputMatrix(9,9);

  outputMatrix = _qt;
  outputMatrix.right_multiply(_dmat);

  _qdmat = outputMatrix;
  _qdmat.right_multiply(_q);
  
}
  


void AnisotropicElasticityTensor::calculateEntries(unsigned int qp)
{
  
  form_r_matrix();
  intialize_material_dt_matrix();
  form_rotational_q_matrix();
  form_transformation_t_matrix();
  form_transformed_material_dmat_matrix(); 
  form_rotated_material_qdmat_matrix();

  int count;

  count = 0;
  
  for(int j = 0; j < 9; j++)
    for(int i = 0; i < 9; i++)
  {
    _values[count] = _qdmat(i,j);
    count = count + 1;
  }
      
  
}


