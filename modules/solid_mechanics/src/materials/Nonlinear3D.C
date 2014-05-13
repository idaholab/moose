#include "Nonlinear3D.h"

#include "SolidModel.h"

#include "Problem.h"
#include "SymmIsotropicElasticityTensor.h"
#include "VolumetricModel.h"

namespace SolidMechanics
{

Nonlinear3D::Nonlinear3D( SolidModel & solid_model,
                          const std::string & name,
                          InputParameters parameters )
  :Element( solid_model, name, parameters ),
   _grad_disp_x(coupledGradient("disp_x")),
   _grad_disp_y(coupledGradient("disp_y")),
   _grad_disp_z(coupledGradient("disp_z")),
   _grad_disp_x_old(coupledGradientOld("disp_x")),
   _grad_disp_y_old(coupledGradientOld("disp_y")),
   _grad_disp_z_old(coupledGradientOld("disp_z")),
   _decomp_method( RashidApprox ),
   _incremental_rotation(3,3),
   _Uhat(3,3)
{

  std::string increment_calculation = solid_model.getParam<std::string>("increment_calculation");
  std::transform( increment_calculation.begin(), increment_calculation.end(),
                  increment_calculation.begin(), ::tolower );
  if ( increment_calculation == "rashidapprox" )
  {
    _decomp_method = RashidApprox;
  }
  else if ( increment_calculation == "eigen" )
  {
    _decomp_method = Eigen;
  }
  else
  {
    mooseError( "The options for the increment calculation are RashidApprox and Eigen.");
  }

}

////////////////////////////////////////////////////////////////////////

Nonlinear3D::~Nonlinear3D()
{
}

////////////////////////////////////////////////////////////////////////

void
Nonlinear3D::computeIncrementalDeformationGradient( std::vector<ColumnMajorMatrix> & Fhat )
{
  // A = grad(u(k+1) - u(k))
  // Fbar = 1 + grad(u(k))
  // Fhat = 1 + A*(Fbar^-1)
  ColumnMajorMatrix A;
  ColumnMajorMatrix Fbar;
  ColumnMajorMatrix Fbar_inverse;
  ColumnMajorMatrix Fhat_average;
  Real volume(0);

  _Fbar.resize(_solid_model.qrule()->n_points());

  for ( unsigned qp= 0; qp < _solid_model.qrule()->n_points(); ++qp )
  {
    fillMatrix( qp, _grad_disp_x, _grad_disp_y, _grad_disp_z, A );
    fillMatrix( qp, _grad_disp_x_old, _grad_disp_y_old, _grad_disp_z_old, Fbar);

    A -= Fbar;

    Fbar.addDiag( 1 );

    _Fbar[qp] = Fbar;


    // Get Fbar^(-1)
    // Computing the inverse is generally a bad idea.
    // It's better to compute LU factors.   For now at least, we'll take
    // a direct route.

    invertMatrix( Fbar, Fbar_inverse );

    Fhat[qp] = A * Fbar_inverse;
    Fhat[qp].addDiag( 1 );


    // Now include the contribution for the integration of Fhat over the element
    Fhat_average += Fhat[qp] * _solid_model.JxW(qp);

    volume += _solid_model.JxW(qp);  // Accumulate original configuration volume
  }

  Fhat_average /= volume;
  const Real det_Fhat_average( detMatrix( Fhat_average ) );
  const Real third( 1./3. );


  // Finalize volumetric locking correction
  for ( unsigned qp=0; qp < _solid_model.qrule()->n_points(); ++qp )
  {
    const Real det_Fhat( detMatrix( Fhat[qp] ) );
    const Real factor( std::pow( det_Fhat_average/det_Fhat, third ) );

    Fhat[qp] *= factor;

  }
//    Moose::out << "Fhat(0,0)" << Fhat[0](0,0) << std::endl;
}

////////////////////////////////////////////////////////////////////////

void
Nonlinear3D::computeDeformationGradient( unsigned int qp, ColumnMajorMatrix & F)
{
  mooseAssert(F.n() == 3 && F.m() == 3, "computeDefGrad requires 3x3 matrix");

  F(0,0) = _grad_disp_x[qp](0) + 1;
  F(0,1) = _grad_disp_x[qp](1);
  F(0,2) = _grad_disp_x[qp](2);
  F(1,0) = _grad_disp_y[qp](0);
  F(1,1) = _grad_disp_y[qp](1) + 1;
  F(1,2) = _grad_disp_y[qp](2);
  F(2,0) = _grad_disp_z[qp](0);
  F(2,1) = _grad_disp_z[qp](1);
  F(2,2) = _grad_disp_z[qp](2) + 1;
}

//////////////////////////////////////////////////////////////////////////

Real
Nonlinear3D::volumeRatioOld(unsigned int qp) const
{
  ColumnMajorMatrix Fnm1T(_grad_disp_x_old[qp],
                          _grad_disp_y_old[qp],
                          _grad_disp_z_old[qp]);
  Fnm1T(0,0) += 1;
  Fnm1T(1,1) += 1;
  Fnm1T(2,2) += 1;

  return detMatrix(Fnm1T);
}

//////////////////////////////////////////////////////////////////////////

void
Nonlinear3D::computeStrainAndRotationIncrement( const ColumnMajorMatrix & Fhat,
                                                SymmTensor & strain_increment )
{
  if ( _decomp_method == RashidApprox )
  {
    computeStrainIncrement( Fhat, strain_increment );
    computePolarDecomposition( Fhat );
  }

  else if ( _decomp_method == Eigen )
 {
 ////
 ////   const int ND = 3;
 ////
 ////   ColumnMajorMatrix eigen_value(ND,1), eigen_vector(ND,ND);
 ////   ColumnMajorMatrix invUhat(ND,ND), logVhat(ND,ND);
 ////   ColumnMajorMatrix n1(ND,1), n2(ND,1), n3(ND,1), N1(ND,1), N2(ND,1), N3(ND,1);
 ////
 ////   ColumnMajorMatrix Chat = Fhat.transpose() * Fhat;
 ////
 ////   Chat.eigen(eigen_value,eigen_vector);
 ////
 ////   for (int i = 0; i < ND; i++)
 ////   {
 ////     N1(i) = eigen_vector(i,0);
 ////     N2(i) = eigen_vector(i,1);
 ////     N3(i) = eigen_vector(i,2);
 ////   }
 ////
 ////   const Real lamda1 = std::sqrt(eigen_value(0));
 ////   const Real lamda2 = std::sqrt(eigen_value(1));
 ////   const Real lamda3 = std::sqrt(eigen_value(2));
 ////
 ////
 ////   const Real log1 = std::log(lamda1);
 ////   const Real log2 = std::log(lamda2);
 ////   const Real log3 = std::log(lamda3);
 ////
 ////   _Uhat = N1 * N1.transpose() * lamda1 +  N2 * N2.transpose() * lamda2 +  N3 * N3.transpose() * lamda3;
 ////
 ////   invertMatrix(_Uhat,invUhat);
 ////
 ////   _incremental_rotation = Fhat * invUhat;
 ////
 ////   strain_increment = N1 * N1.transpose() * log1 +  N2 * N2.transpose() * log2 +  N3 * N3.transpose() * log3;

   Element::polarDecompositionEigen( Fhat, _incremental_rotation, strain_increment);


  }
  else
  {
    mooseError("Unknown polar decomposition type!");
  }
}

////////////////////////////////////////////////////////////////////////

void
Nonlinear3D::computeStrainIncrement( const ColumnMajorMatrix & Fhat,
                                     SymmTensor & strain_increment )
{

  //
  // A calculation of the strain at the mid-interval is probably more
  // accurate (second vs. first order).  This would require the
  // incremental deformation gradient at the mid-step, which we
  // currently don't have.  We would then have to calculate the
  // rotation for the whole step.
  //
  //
  // We are looking for:
  //     log( Uhat )
  //  =  log( sqrt( Fhat^T*Fhat ) )
  //  =  log( sqrt( Chat ) )
  // A Taylor series expansion gives:
  //     ( Chat - 0.25 * Chat^T*Chat - 0.75 * I )
  //  =  ( - 0.25 * Chat^T*Chat + Chat - 0.75 * I )
  //  =  ( (0.25*Chat - 0.75*I) * (Chat - I) )
  //  =  ( B * A )
  //    B
  //  = 0.25*Chat - 0.75*I
  //  = 0.25*(Chat - I) - 0.5*I
  //  = 0.25*A - 0.5*I
  //

  const Real Uxx = Fhat(0,0);
  const Real Uxy = Fhat(0,1);
  const Real Uxz = Fhat(0,2);
  const Real Uyx = Fhat(1,0);
  const Real Uyy = Fhat(1,1);
  const Real Uyz = Fhat(1,2);
  const Real Uzx = Fhat(2,0);
  const Real Uzy = Fhat(2,1);
  const Real Uzz = Fhat(2,2);

  const Real Axx = Uxx*Uxx + Uyx*Uyx + Uzx*Uzx - 1.0;
  const Real Axy = Uxx*Uxy + Uyx*Uyy + Uzx*Uzy;
  const Real Axz = Uxx*Uxz + Uyx*Uyz + Uzx*Uzz;
  const Real Ayy = Uxy*Uxy + Uyy*Uyy + Uzy*Uzy - 1.0;
  const Real Ayz = Uxy*Uxz + Uyy*Uyz + Uzy*Uzz;
  const Real Azz = Uxz*Uxz + Uyz*Uyz + Uzz*Uzz - 1.0;

  const Real Bxx = 0.25 * Axx - 0.5;
  const Real Bxy = 0.25 * Axy;
  const Real Bxz = 0.25 * Axz;
  const Real Byy = 0.25 * Ayy - 0.5;
  const Real Byz = 0.25 * Ayz;
  const Real Bzz = 0.25 * Azz - 0.5;

  strain_increment.xx( -(Bxx*Axx + Bxy*Axy + Bxz*Axz) );
  strain_increment.xy( -(Bxx*Axy + Bxy*Ayy + Bxz*Ayz) );
  strain_increment.zx( -(Bxx*Axz + Bxy*Ayz + Bxz*Azz) );
  strain_increment.yy( -(Bxy*Axy + Byy*Ayy + Byz*Ayz) );
  strain_increment.yz( -(Bxy*Axz + Byy*Ayz + Byz*Azz) );
  strain_increment.zz( -(Bxz*Axz + Byz*Ayz + Bzz*Azz) );

}

////////////////////////////////////////////////////////////////////////

void
Nonlinear3D::computePolarDecomposition( const ColumnMajorMatrix & Fhat )
{

  // From Rashid, 1993.
  ColumnMajorMatrix Fhat_inverse;
  invertMatrix( Fhat, Fhat_inverse );
  Fhat_inverse = Fhat;
  /*Moose::out << "Fhat = " << std::endl;
  ColumnMajorMatrix out(Fhat);
  out.print();
  Moose::out << "Fhat_inverse = " << std::endl;
  out = Fhat_inverse;
  out.print();*/

  const Real Uxx = Fhat_inverse(0,0);
  const Real Uxy = Fhat_inverse(0,1);
  const Real Uxz = Fhat_inverse(0,2);
  const Real Uyx = Fhat_inverse(1,0);
  const Real Uyy = Fhat_inverse(1,1);
  const Real Uyz = Fhat_inverse(1,2);
  const Real Uzx = Fhat_inverse(2,0);
  const Real Uzy = Fhat_inverse(2,1);
  const Real Uzz = Fhat_inverse(2,2);

  const Real Ax = Uyz - Uzy;
  const Real Ay = Uzx - Uxz;
  const Real Az = Uxy - Uyx;
  const Real Q = 0.25 * (Ax*Ax + Ay*Ay + Az*Az);
  const Real traceF = Uxx + Uyy + Uzz;
  const Real P = 0.25 * (traceF - 1) * (traceF - 1);
  const Real Y = 1 / ((Q+P)*(Q+P)*(Q+P));

  const Real C1 = std::sqrt(P * (1 + (P*(Q+Q+(Q+P))) * (1-(Q+P)) * Y));
  const Real C2 = 0.125 + Q * 0.03125 * (P*P - 12*(P-1)) / (P*P);
  const Real C3 = 0.5 * std::sqrt( (P*Q*(3-Q) + P*P*P + Q*Q) * Y );

  // Since the input to this routine is the incremental deformation gradient
  //   and not the inverse incremental gradient, this result is the transpose
  //   of the one in Rashid's paper.
  _incremental_rotation(0,0) = C1 + (C2*Ax)*Ax;
  _incremental_rotation(0,1) =      (C2*Ay)*Ax + (C3*Az);
  _incremental_rotation(0,2) =      (C2*Az)*Ax - (C3*Ay);
  _incremental_rotation(1,0) =      (C2*Ax)*Ay - (C3*Az);
  _incremental_rotation(1,1) = C1 + (C2*Ay)*Ay;
  _incremental_rotation(1,2) =      (C2*Az)*Ay + (C3*Ax);
  _incremental_rotation(2,0) =      (C2*Ax)*Az + (C3*Ay);
  _incremental_rotation(2,1) =      (C2*Ay)*Az - (C3*Ax);
  _incremental_rotation(2,2) = C1 + (C2*Az)*Az;

}

////////////////////////////////////////////////////////////////////////

void
Nonlinear3D::finalizeStress( std::vector<SymmTensor*> & t)
{
  // Using the incremental rotation, update the stress to the current configuration (R*T*R^T)
  for (unsigned i(0); i < t.size(); ++i)
  {
    Element::rotateSymmetricTensor( _incremental_rotation, *t[i], *t[i]);
  }

}

////////////////////////////////////////////////////////////////////////

void
Nonlinear3D::computeStrain( const unsigned qp,
                            const SymmTensor & total_strain_old,
                            SymmTensor & total_strain_new,
                            SymmTensor & strain_increment )
{
  computeStrainAndRotationIncrement(_Fhat[qp], strain_increment);

  total_strain_new = strain_increment;
  total_strain_new += total_strain_old;
}

////////////////////////////////////////////////////////////////////////

void
Nonlinear3D::init()
{
  _Fhat.resize(_solid_model.qrule()->n_points());

  computeIncrementalDeformationGradient(_Fhat);
}

////////////////////////////////////////////////////////////////////////

}
