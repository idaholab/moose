#include "MaterialModel.h"

#include "Problem.h"
#include "SymmIsotropicElasticityTensor.h"
#include "VolumetricModel.h"

template<>
InputParameters validParams<MaterialModel>()
{
  InputParameters params = validParams<SolidModel>();
  params.addRequiredCoupledVar("disp_x", "The x displacement");
  params.addRequiredCoupledVar("disp_y", "The y displacement");
  params.addRequiredCoupledVar("disp_z", "The z displacement");
  params.addParam<std::string>("increment_calculation", "RashidApprox", "The algorithm to use when computing the incremental strain and rotation (RashidApprox or Eigen).");
  return params;
}



MaterialModel::MaterialModel( const std::string & name,
                              InputParameters parameters )
  :SolidModel( name, parameters )
{
  SymmIsotropicElasticityTensor * iso =  new SymmIsotropicElasticityTensor;
  iso->setLambda( _lambda );
  iso->setShearModulus( _shear_modulus );
  iso->calculate(0);
  elasticityTensor( iso );

}

////////////////////////////////////////////////////////////////////////

MaterialModel::~MaterialModel()
{
}

////////////////////////////////////////////////////////////////////////

void
MaterialModel::computeStress()
{
  // Given the stretching, compute the stress increment and add it to the old stress.
  // stress = stressOld + stressIncrement
//   const Real lamda( 0 );
//   const Real twoG( 10e6 );
//   const Real traceD( strain_increment(0,0) +
//                      strain_increment(1,1) +
//                      strain_increment(2,2) );
//   const Real bulk_update = lamda * traceD;
//   _stress[_qp](0,0) = _stress_old[_qp](0,0) + ( twoG * strain_increment(0,0) + bulk_update );
//   _stress[_qp](1,1) = _stress_old[_qp](1,1) + ( twoG * strain_increment(1,1) + bulk_update );
//   _stress[_qp](2,2) = _stress_old[_qp](2,2) + ( twoG * strain_increment(2,2) + bulk_update );
//   _stress[_qp](0,1) = _stress_old[_qp](0,1) + ( twoG * strain_increment(0,1)               );
//   _stress[_qp](0,2) = _stress_old[_qp](0,2) + ( twoG * strain_increment(0,2)               );
//   _stress[_qp](1,2) = _stress_old[_qp](1,2) + ( twoG * strain_increment(1,2)               );
//   _stress[_qp](1,0) = _stress[_qp](0,1);
//   _stress[_qp](2,0) = _stress[_qp](0,2);
//   _stress[_qp](2,1) = _stress[_qp](1,2);

  SymmTensor stress_new( _elasticity_tensor[_qp] * _strain_increment );
  _stress[_qp] = stress_new;
  _stress[_qp] += _stress_old;

  //   std::cout << "ELASTICITY TENSOR: " << " at time " << _t << "\n";
  //  _elasticity_tensor->print();

//   std::cout << "STRAIN INCREMENT: " << _qp << "\n"
//             << _strain_increment(0,0) << " " << _strain_increment(0,1) << " " << _strain_increment(0,2) << std::endl
//             << _strain_increment(1,0) << " " << _strain_increment(1,1) << " " << _strain_increment(1,2) << std::endl
//             << _strain_increment(2,0) << " " << _strain_increment(2,1) << " " << _strain_increment(2,2) << std::endl;

//   std::cout << "STRESS: " << _qp << " at time " << _t << "\n"
//             << _stress[_qp](0,0) << " " << _stress[_qp](0,1) << " " << _stress[_qp](0,2) << std::endl
//             << _stress[_qp](1,0) << " " << _stress[_qp](1,1) << " " << _stress[_qp](1,2) << std::endl
//             << _stress[_qp](2,0) << " " << _stress[_qp](2,1) << " " << _stress[_qp](2,2) << std::endl;

}

////////////////////////////////////////////////////////////////////////

void
MaterialModel::rotateSymmetricTensor( const ColumnMajorMatrix & R,
                                      const RealTensorValue & T,
                                      RealTensorValue & result )
{

  //     R           T         Rt
  //  00 01 02   00 01 02   00 10 20
  //  10 11 12 * 10 11 12 * 01 11 21
  //  20 21 22   20 21 22   02 12 22
  //
  const Real T00 = R(0,0)*T(0,0) + R(0,1)*T(1,0) + R(0,2)*T(2,0);
  const Real T01 = R(0,0)*T(0,1) + R(0,1)*T(1,1) + R(0,2)*T(2,1);
  const Real T02 = R(0,0)*T(0,2) + R(0,1)*T(1,2) + R(0,2)*T(2,2);

  const Real T10 = R(1,0)*T(0,0) + R(1,1)*T(1,0) + R(1,2)*T(2,0);
  const Real T11 = R(1,0)*T(0,1) + R(1,1)*T(1,1) + R(1,2)*T(2,1);
  const Real T12 = R(1,0)*T(0,2) + R(1,1)*T(1,2) + R(1,2)*T(2,2);

  const Real T20 = R(2,0)*T(0,0) + R(2,1)*T(1,0) + R(2,2)*T(2,0);
  const Real T21 = R(2,0)*T(0,1) + R(2,1)*T(1,1) + R(2,2)*T(2,1);
  const Real T22 = R(2,0)*T(0,2) + R(2,1)*T(1,2) + R(2,2)*T(2,2);

  result = RealTensorValue(
    T00 * R(0,0) + T01 * R(0,1) + T02 * R(0,2),
    T00 * R(1,0) + T01 * R(1,1) + T02 * R(1,2),
    T00 * R(2,0) + T01 * R(2,1) + T02 * R(2,2),

    T10 * R(0,0) + T11 * R(0,1) + T12 * R(0,2),
    T10 * R(1,0) + T11 * R(1,1) + T12 * R(1,2),
    T10 * R(2,0) + T11 * R(2,1) + T12 * R(2,2),

    T20 * R(0,0) + T21 * R(0,1) + T22 * R(0,2),
    T20 * R(1,0) + T21 * R(1,1) + T22 * R(1,2),
    T20 * R(2,0) + T21 * R(2,1) + T22 * R(2,2) );

}

////////////////////////////////////////////////////////////////////////

void
MaterialModel::rotateSymmetricTensor( const ColumnMajorMatrix & R,
                                      const SymmTensor & T,
                                      SymmTensor & result )
{

  //     R           T         Rt
  //  00 01 02   00 01 02   00 10 20
  //  10 11 12 * 10 11 12 * 01 11 21
  //  20 21 22   20 21 22   02 12 22
  //
  const Real T00 = R(0,0)*T.xx() + R(0,1)*T.xy() + R(0,2)*T.zx();
  const Real T01 = R(0,0)*T.xy() + R(0,1)*T.yy() + R(0,2)*T.yz();
  const Real T02 = R(0,0)*T.zx() + R(0,1)*T.yz() + R(0,2)*T.zz();

  const Real T10 = R(1,0)*T.xx() + R(1,1)*T.xy() + R(1,2)*T.zx();
  const Real T11 = R(1,0)*T.xy() + R(1,1)*T.yy() + R(1,2)*T.yz();
  const Real T12 = R(1,0)*T.zx() + R(1,1)*T.yz() + R(1,2)*T.zz();

  const Real T20 = R(2,0)*T.xx() + R(2,1)*T.xy() + R(2,2)*T.zx();
  const Real T21 = R(2,0)*T.xy() + R(2,1)*T.yy() + R(2,2)*T.yz();
  const Real T22 = R(2,0)*T.zx() + R(2,1)*T.yz() + R(2,2)*T.zz();

  result.xx( T00 * R(0,0) + T01 * R(0,1) + T02 * R(0,2) );
  result.yy( T10 * R(1,0) + T11 * R(1,1) + T12 * R(1,2) );
  result.zz( T20 * R(2,0) + T21 * R(2,1) + T22 * R(2,2) );
  result.xy( T00 * R(1,0) + T01 * R(1,1) + T02 * R(1,2) );
  result.yz( T10 * R(2,0) + T11 * R(2,1) + T12 * R(2,2) );
  result.zx( T00 * R(2,0) + T01 * R(2,1) + T02 * R(2,2) );

}

////////////////////////////////////////////////////////////////////////


void
MaterialModel::fillMatrix( const VariableGradient & grad_x,
                           const VariableGradient & grad_y,
                           const VariableGradient & grad_z,
                           ColumnMajorMatrix & A )
{
  A(0,0) = grad_x[_qp](0); A(0,1) = grad_x[_qp](1); A(0,2) = grad_x[_qp](2);
  A(1,0) = grad_y[_qp](0); A(1,1) = grad_y[_qp](1); A(1,2) = grad_y[_qp](2);
  A(2,0) = grad_z[_qp](0); A(2,1) = grad_z[_qp](1); A(2,2) = grad_z[_qp](2);
}

////////////////////////////////////////////////////////////////////////

Real
MaterialModel::detMatrix( const ColumnMajorMatrix & A )
{
  Real Axx = A(0,0);
  Real Axy = A(0,1);
  Real Axz = A(0,2);
  Real Ayx = A(1,0);
  Real Ayy = A(1,1);
  Real Ayz = A(1,2);
  Real Azx = A(2,0);
  Real Azy = A(2,1);
  Real Azz = A(2,2);

  return   Axx*Ayy*Azz + Axy*Ayz*Azx + Axz*Ayx*Azy
         - Azx*Ayy*Axz - Azy*Ayz*Axx - Azz*Ayx*Axy;
}

////////////////////////////////////////////////////////////////////////

void
MaterialModel::invertMatrix( const ColumnMajorMatrix & A,
                             ColumnMajorMatrix & Ainv )
{
  Real Axx = A(0,0);
  Real Axy = A(0,1);
  Real Axz = A(0,2);
  Real Ayx = A(1,0);
  Real Ayy = A(1,1);
  Real Ayz = A(1,2);
  Real Azx = A(2,0);
  Real Azy = A(2,1);
  Real Azz = A(2,2);

  mooseAssert( detMatrix( A ) > 0, "Matrix is not positive definite!" );
  Real detInv = 1 / detMatrix( A );

  Ainv(0,0) = +(Ayy*Azz-Azy*Ayz) * detInv;
  Ainv(0,1) = -(Axy*Azz-Azy*Axz) * detInv;
  Ainv(0,2) = +(Axy*Ayz-Ayy*Axz) * detInv;
  Ainv(1,0) = -(Ayx*Azz-Azx*Ayz) * detInv;
  Ainv(1,1) = +(Axx*Azz-Azx*Axz) * detInv;
  Ainv(1,2) = -(Axx*Ayz-Ayx*Axz) * detInv;
  Ainv(2,0) = +(Ayx*Azy-Azx*Ayy) * detInv;
  Ainv(2,1) = -(Axx*Azy-Azx*Axy) * detInv;
  Ainv(2,2) = +(Axx*Ayy-Ayx*Axy) * detInv;
}

////////////////////////////////////////////////////////////////////////

int
MaterialModel::delta(int i, int j)
{

  if(i == j)
    return 1;
  else
    return 0;
}

////////////////////////////////////////////////////////////////////////
