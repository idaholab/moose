#include "MaterialModel.h"

#include "IsotropicElasticityTensor.h"
#include "VolumetricModel.h"

#include "MooseSystem.h"

template<>
InputParameters validParams<MaterialModel>()
{
  InputParameters params = validParams<Material>();
  params.addParam<Real>("bulk_modulus", "The bulk modulus for the material.");
  params.addParam<Real>("lambda", "Lame's first parameter for the material.");
  params.addParam<Real>("poissons_ratio", "Poisson's ratio for the material");
  params.addParam<Real>("shear_modulus", "The shear modulus of the material.");
  params.addParam<Real>("youngs_modulus", "Young's modulus of the material.");
  params.addRequiredCoupledVar("disp_x", "The x displacement");
  params.addCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");
  params.addParam<std::string>("increment_calculation", "RashidApprox", "The algorithm to use when computing the incremental strain and rotation (RashidApprox or Eigen).");
  params.addParam<Real>("thermal_expansion", 0.0, "The thermal expansion coefficient.");
  params.addCoupledVar("temp", "Coupled Temperature");
  return params;
}



MaterialModel::MaterialModel( const std::string & name,
                              InputParameters parameters )
  :Material( name, parameters ),
   _initialized(false),
   _bulk_modulus_set( parameters.isParamValid("bulk_modulus") ),
   _lambda_set( parameters.isParamValid("lambda") ),
   _poissons_ratio_set( parameters.isParamValid("poissons_ratio") ),
   _shear_modulus_set( parameters.isParamValid("shear_modulus") ),
   _youngs_modulus_set( parameters.isParamValid("youngs_modulus") ),
   _bulk_modulus( _bulk_modulus_set ? getParam<Real>("bulk_modulus") : -1 ),
   _lambda( _lambda_set ? getParam<Real>("lambda") : -1 ),
   _poissons_ratio( _poissons_ratio_set ?  getParam<Real>("poissons_ratio") : -1 ),
   _shear_modulus( _shear_modulus_set ? getParam<Real>("shear_modulus") : -1 ),
   _youngs_modulus( _youngs_modulus_set ? getParam<Real>("youngs_modulus") : -1 ),
   _grad_disp_x(coupledGradient("disp_x")),
   _grad_disp_y(_dim > 1 ? coupledGradient("disp_y") : _grad_zero),
   _grad_disp_z(_dim > 2 ? coupledGradient("disp_z") : _grad_zero),
   _grad_disp_x_old(coupledGradientOld("disp_x")),
   _grad_disp_y_old(_dim > 1 ? coupledGradientOld("disp_y") : _grad_zero),
   _grad_disp_z_old(_dim > 2 ? coupledGradientOld("disp_z") : _grad_zero),
   _has_temp(isCoupled("temp")),
   _temperature(_has_temp ? coupledValue("temp") : _zero),
   _temperature_old(_has_temp ? coupledValueOld("temp") : _zero),
   _volumetric_models(0),
   _decomp_method( RashidApprox ),
   _alpha(getParam<Real>("thermal_expansion")),
   _stress(declareProperty<RealTensorValue>("stress")),
   _stress_old(declarePropertyOld<RealTensorValue>("stress")),
   _Jacobian_mult(declareProperty<ColumnMajorMatrix>("Jacobian_mult")),
   _strain_increment(3,3),
   _incremental_rotation(3,3),
   _Uhat(3,3),
   _elasticity_tensor(NULL)
{
//   std::cout << "TESTING MaterialModel class..." << std::endl;
//   testMe();
  int num_elastic_constants =
    _bulk_modulus_set + _lambda_set + _poissons_ratio_set + _shear_modulus_set + _youngs_modulus_set;

  if ( num_elastic_constants != 2 )
  {
    std::string err("Exactly two elastic constants must be defined for material '");
    err += name;
    err += "'.";
    mooseError(err);
  }

  if ( _bulk_modulus_set && _bulk_modulus <= 0 )
  {
    std::string err("Bulk modulus must be positive in material '");
    err += name;
    err += "'.";
    mooseError(err);
  }
  if ( _poissons_ratio_set && (_poissons_ratio <= -1.0 || _poissons_ratio >= 0.5) )
  {
    std::string err("Poissons ratio must be greater than -1 and less than 0.5 in material '");
    err += name;
    err += "'.";
    mooseError(err);
  }
  if ( _shear_modulus_set &&  _shear_modulus < 0 )
  {
    std::string err("Shear modulus must not be negative in material '");
    err += name;
    err += "'.";
    mooseError(err);
  }
  if ( _youngs_modulus_set &&  _youngs_modulus <= 0 )
  {
    std::string err("Youngs modulus must be positive in material '");
    err += name;
    err += "'.";
    mooseError(err);
  }

  // Calculate lambda, the shear modulus, and Young's modulus
  if(_lambda_set && _shear_modulus_set) // First and second Lame
  {
    _youngs_modulus = _shear_modulus*(3*_lambda+2*_shear_modulus)/(_lambda+_shear_modulus);
  }
  else if(_lambda_set && _poissons_ratio_set)
  {
    _shear_modulus = (_lambda * (1.0 - 2.0 * _poissons_ratio)) / (2.0 * _poissons_ratio);
    _youngs_modulus = _shear_modulus*(3*_lambda+2*_shear_modulus)/(_lambda+_shear_modulus);

  }
  else if(_lambda_set && _bulk_modulus_set)
  {
    _shear_modulus = 3.0 * (_bulk_modulus - _lambda) / 2.0;
    _youngs_modulus = _shear_modulus*(3*_lambda+2*_shear_modulus)/(_lambda+_shear_modulus);
  }
  else if(_lambda_set && _youngs_modulus_set)
  {
    _shear_modulus = ( (_youngs_modulus - 3.0*_lambda) / 4.0 ) + ( std::sqrt( (_youngs_modulus-3.0*_lambda)*(_youngs_modulus-3.0*_lambda) + 8.0*_lambda*_youngs_modulus ) / 4.0 );
  }
  else if(_shear_modulus_set && _poissons_ratio_set)
  {
    _lambda = ( 2.0 * _shear_modulus * _poissons_ratio ) / (1.0 - 2.0*_poissons_ratio);
    _youngs_modulus = _shear_modulus*(3*_lambda+2*_shear_modulus)/(_lambda+_shear_modulus);
  }
  else if(_shear_modulus_set && _bulk_modulus_set)
  {
    _lambda = _bulk_modulus - 2.0 * _shear_modulus / 3.0;
    _youngs_modulus = _shear_modulus*(3*_lambda+2*_shear_modulus)/(_lambda+_shear_modulus);
  }
  else if(_shear_modulus_set && _youngs_modulus_set)
  {
    _lambda = ((2.0*_shear_modulus - _youngs_modulus) * _shear_modulus) / (_youngs_modulus - 3.0*_shear_modulus);
  }
  else if(_poissons_ratio_set && _bulk_modulus_set)
  {
    _lambda = (3.0 * _bulk_modulus * _poissons_ratio) / (1.0 + _poissons_ratio);
    _shear_modulus = (3.0 * _bulk_modulus * (1.0 - 2.0*_poissons_ratio)) / (2.0 * (1.0 + _poissons_ratio));
    _youngs_modulus = _shear_modulus*(3*_lambda+2*_shear_modulus)/(_lambda+_shear_modulus);
  }
  else if(_youngs_modulus_set && _poissons_ratio_set) // Young's Modulus and Poisson's Ratio
  {
    _lambda = (_poissons_ratio * _youngs_modulus) / ( (1.0+_poissons_ratio) * (1-2.0*_poissons_ratio) );
    _shear_modulus = _youngs_modulus / ( 2.0 * (1.0+_poissons_ratio));
  }
  else if(_youngs_modulus_set && _bulk_modulus_set)
  {
    _lambda = 3.0 * _bulk_modulus * (3.0 * _bulk_modulus - _youngs_modulus) / (9.0 * _bulk_modulus - _youngs_modulus);
    _shear_modulus = 3.0 * _youngs_modulus * _bulk_modulus / (9.0 * _bulk_modulus - _youngs_modulus);
  }

  _lambda_set = true;
  _shear_modulus_set = true;
  _youngs_modulus_set = true;

  IsotropicElasticityTensor * iso =  new IsotropicElasticityTensor;
  iso->setLambda( _lambda );
  iso->setShearModulus( _shear_modulus );
  iso->calculate(0);
  elasticityTensor( iso );

  std::string increment_calculation = getParam<std::string>("increment_calculation");
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


  //   std::cout << "ELASTICITY TENSOR: " << " at time " << _t << "\n";
  // _elasticity_tensor->print();


}

////////////////////////////////////////////////////////////////////////

MaterialModel::~MaterialModel()
{
  delete _elasticity_tensor;
}

////////////////////////////////////////////////////////////////////////

void
MaterialModel::elasticityTensor( ElasticityTensor * e )
{
  delete _elasticity_tensor;
  _elasticity_tensor = e;
}

////////////////////////////////////////////////////////////////////////

void
MaterialModel::computeIncrementalDeformationGradient( std::vector<ColumnMajorMatrix> & Fhat )
{
  // A = grad(u(k+1) - u(k))
  // Fbar = 1 + grad(u(k))
  // Fhat = 1 + A*(Fbar^-1)
  ColumnMajorMatrix A;
  ColumnMajorMatrix Fbar;
  ColumnMajorMatrix Fbar_inverse;
  ColumnMajorMatrix Fhat_average;
  Real volume(0);

  _Fbar.resize(_n_qpoints);

  for ( _qp= 0; _qp < _n_qpoints; ++_qp )
  {
    fillMatrix( _grad_disp_x, _grad_disp_y, _grad_disp_z, A );
    fillMatrix( _grad_disp_x_old, _grad_disp_y_old, _grad_disp_z_old, Fbar);

    A -= Fbar;

    Fbar.addDiag( 1 );

    _Fbar[_qp] = Fbar;


    // Get Fbar^(-1)
    // Computing the inverse is generally a bad idea.
    // It's better to compute LU factors.   For now at least, we'll take
    // a direct route.

    invertMatrix( Fbar, Fbar_inverse );

    Fhat[_qp] = A * Fbar_inverse;
    Fhat[_qp].addDiag( 1 );


    // Now include the contribution for the integration of Fhat over the element
    Fhat_average += Fhat[_qp] * _JxW[_qp];

    volume += _JxW[_qp];  // Accumulate original configuration volume
  }

  Fhat_average /= volume;
  const Real det_Fhat_average( detMatrix( Fhat_average ) );
  const Real third( 1./3. );


  // Finalize volumetric locking correction
  for ( _qp=0; _qp < _n_qpoints; ++_qp )
  {
    const Real det_Fhat( detMatrix( Fhat[_qp] ) );
    const Real factor( std::pow( det_Fhat_average/det_Fhat, third ) );

    Fhat[_qp] *= factor;

  }
}

////////////////////////////////////////////////////////////////////////

void
MaterialModel::computeStrainAndRotationIncrement( const ColumnMajorMatrix & Fhat)
{
  if ( _decomp_method == RashidApprox )
  {
    computeStrainIncrement(Fhat);
    computePolarDecomposition( Fhat);
  }

  else if ( _decomp_method == Eigen )
  {

    const int ND = 3;

    ColumnMajorMatrix eigen_value(ND,1), eigen_vector(ND,ND);
    ColumnMajorMatrix invUhat(ND,ND), logVhat(ND,ND), logUhat(ND,ND);
    ColumnMajorMatrix n1(ND,1), n2(ND,1), n3(ND,1), N1(ND,1), N2(ND,1), N3(ND,1);

    ColumnMajorMatrix Chat = Fhat.transpose() * Fhat;

    Chat.eigen(eigen_value,eigen_vector);

    for(int i = 0; i < ND; i++)
    {
      N1(i) = eigen_vector(i,0);
      N2(i) = eigen_vector(i,1);
      N3(i) = eigen_vector(i,2);
    }

    const Real lamda1 = std::sqrt(eigen_value(0));
    const Real lamda2 = std::sqrt(eigen_value(1));
    const Real lamda3 = std::sqrt(eigen_value(2));


    const Real log1 = std::log(lamda1);
    const Real log2 = std::log(lamda2);
    const Real log3 = std::log(lamda3);

    _Uhat = N1 * N1.transpose() * lamda1 +  N2 * N2.transpose() * lamda2 +  N3 * N3.transpose() * lamda3;

    invertMatrix(_Uhat,invUhat);

    _incremental_rotation = Fhat * invUhat;

    logUhat = N1 * N1.transpose() * log1 +  N2 * N2.transpose() * log2 +  N3 * N3.transpose() * log3;

    _strain_increment = logUhat * (1.0 / _dt);

    /*
    n1 = _incremental_rotation * N1;
    n2 = _incremental_rotation * N2;
    n3 = _incremental_rotation * N3;


    logVhat = n1 * n1.transpose() * log1 +  n2 * n2.transpose() * log2 +  n3 * n3.transpose() * log3;

    _strain_increment = logVhat * (1.0 / _dt);*/
  }
  else
  {
    mooseError("Unknown polar decomposition type!");
  }
}

////////////////////////////////////////////////////////////////////////

void
MaterialModel::computeStrainIncrement( const ColumnMajorMatrix & Fhat)
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
  //     1/dt * log( Uhat )
  //  =  1/dt * log( sqrt( Fhat^T*Fhat ) )
  //  =  1/dt * log( sqrt( Chat ) )
  // A Taylor series expansion gives:
  //     1/dt * ( Chat - 0.25 * Chat^T*Chat - 0.75 * I )
  //  =  1/dt * ( - 0.25 * Chat^T*Chat + Chat - 0.75 * I )
  //  = -1/dt * ( (0.25*Chat - 0.75*I) * (Chat - I) )
  //  = -1/dt * ( B * A )
  //    B
  //  = 0.25*Chat - 0.75*I
  //  = 0.25*(Chat - I) - 0.5*I
  //  = 0.25*A - 0.5*I
  //

  const Real dt_inv = _dt ? 1 / _dt : 1;

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

  _strain_increment(0,0) = -(Bxx*Axx + Bxy*Axy + Bxz*Axz) * dt_inv;
  _strain_increment(0,1) = -(Bxx*Axy + Bxy*Ayy + Bxz*Ayz) * dt_inv;
  _strain_increment(0,2) = -(Bxx*Axz + Bxy*Ayz + Bxz*Azz) * dt_inv;
  _strain_increment(1,1) = -(Bxy*Axy + Byy*Ayy + Byz*Ayz) * dt_inv;
  _strain_increment(1,2) = -(Bxy*Axz + Byy*Ayz + Byz*Azz) * dt_inv;
  _strain_increment(2,2) = -(Bxz*Axz + Byz*Ayz + Bzz*Azz) * dt_inv;
  _strain_increment(1,0) = _strain_increment(0,1);
  _strain_increment(2,0) = _strain_increment(0,2);
  _strain_increment(2,1) = _strain_increment(1,2);

}

////////////////////////////////////////////////////////////////////////

void
MaterialModel::computePolarDecomposition( const ColumnMajorMatrix & Fhat )
{

  // From Rashid, 1993.
  ColumnMajorMatrix Fhat_inverse;
  invertMatrix( Fhat, Fhat_inverse );
  Fhat_inverse = Fhat;

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
  const Real C3 = 0.5 * std::sqrt( (P*Q*(3-Q) + P*P*P + Q*Q) / (P+Q)*(P+Q)*(P+Q) );

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
MaterialModel::modifyStrain()
{
  if ( _has_temp && _t_step != 0 )
  {
    ColumnMajorMatrix tStrain;
    tStrain.setDiag( _alpha/_dt * (_temperature[_qp] - _temperature_old[_qp]) );
    _strain_increment -= tStrain;
  }
  for (unsigned int i(0); i < _volumetric_models.size(); ++i)
  {
    _volumetric_models[i]->modifyStrain(_qp, _strain_increment);
  }
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
//   _stress[_qp](0,0) = _stress_old[_qp](0,0) + _dt * ( twoG * strain_increment(0,0) + bulk_update );
//   _stress[_qp](1,1) = _stress_old[_qp](1,1) + _dt * ( twoG * strain_increment(1,1) + bulk_update );
//   _stress[_qp](2,2) = _stress_old[_qp](2,2) + _dt * ( twoG * strain_increment(2,2) + bulk_update );
//   _stress[_qp](0,1) = _stress_old[_qp](0,1) + _dt * ( twoG * strain_increment(0,1)               );
//   _stress[_qp](0,2) = _stress_old[_qp](0,2) + _dt * ( twoG * strain_increment(0,2)               );
//   _stress[_qp](1,2) = _stress_old[_qp](1,2) + _dt * ( twoG * strain_increment(1,2)               );
//   _stress[_qp](1,0) = _stress[_qp](0,1);
//   _stress[_qp](2,0) = _stress[_qp](0,2);
//   _stress[_qp](2,1) = _stress[_qp](1,2);

  //
  // This is more work than needs to be done.  The strain and stress tensors are symmetric, and so we are carrying
  //   a third more memory than is required.  We are also running a 9x9 * 9x1 matrix-vector multiply when at most
  //   a 6x6 * 6x1 matrix vector multiply is needed.  For the most common case, isotropic elasticity, only two
  //   constants are needed and a matrix vector multiply can be avoided entirely.
  //

  _strain_increment.reshape(9, 1);
  ColumnMajorMatrix stress_new( *_elasticity_tensor * _strain_increment );
  _strain_increment.reshape(3, 3);
  stress_new *= _dt;
  stress_new.fill(_stress[_qp]);
  _stress[_qp] += _stress_old[_qp];

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
MaterialModel::finalizeStress()
{
  // Using the incremental rotation, update the stress to the current configuration (R*T*R^T)
  _stress[_qp] = rotateSymmetricTensor( _incremental_rotation, _stress[_qp] );

//   std::cout << "STRESS: " << _qp << "\n"
//             << _stress[_qp](0,0) << " " << _stress[_qp](0,1) << " " << _stress[_qp](0,2) << std::endl
//             << _stress[_qp](1,0) << " " << _stress[_qp](1,1) << " " << _stress[_qp](1,2) << std::endl
//             << _stress[_qp](2,0) << " " << _stress[_qp](2,1) << " " << _stress[_qp](2,2) << std::endl;

}

////////////////////////////////////////////////////////////////////////

RealTensorValue
MaterialModel::rotateSymmetricTensor( const ColumnMajorMatrix & R,
                                      const RealTensorValue & T )
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

  return RealTensorValue(
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
MaterialModel::computeProperties()
{
  // Compute the stretching to be handed to the constitutive evaluation
  // Handle volumetric locking along the way

  _Fhat.resize(_n_qpoints);

  computeIncrementalDeformationGradient(_Fhat);


  for ( _qp = 0; _qp < _n_qpoints; ++_qp )
  {

    computeStrainAndRotationIncrement(_Fhat[_qp]);
    modifyStrain();


    computeStress();
    finalizeStress();
    computePreconditioning();

  }
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

void
MaterialModel::testMe()
{
  ColumnMajorMatrix fred;
//  const Real s = std::sin(3*M_PI/180);
//  const Real c = std::cos(3*M_PI/180);
//   fred(0,0) =  c;
//   fred(0,1) = -s;
//   fred(0,2) =  0;
//   fred(1,0) =  s;
//   fred(1,1) =  c;
//   fred(1,2) =  0;
//   fred(2,0) =  0;
//   fred(2,1) =  0;
//   fred(2,2) =  1;

//   fred(0,0) =  1;
//   fred(0,1) =  0.001;
//   fred(0,2) =  0;
//   fred(1,0) =  0;
//   fred(1,1) =  1;
//   fred(1,2) =  0;
//   fred(2,0) =  0;
//   fred(2,1) =  0;
//   fred(2,2) =  1;

  fred(0,0) =  .25;
  fred(0,1) =  -1.25;
  fred(0,2) =  0;
  fred(1,0) =  2;
  fred(1,1) =  1;
  fred(1,2) =  0;
  fred(2,0) =  0;
  fred(2,1) =  0;
  fred(2,2) =  1;


  _dt = 1;

  ColumnMajorMatrix d;
  computeStrainIncrement(fred);
  ColumnMajorMatrix rot;
  computePolarDecomposition(fred);

  std::cout << std::setprecision(14);
  std::cout << "Input:\n";
  std::cout << fred(0,0) << " " << fred(0,1) << " " << fred(0,2) << std::endl;
  std::cout << fred(1,0) << " " << fred(1,1) << " " << fred(1,2) << std::endl;
  std::cout << fred(2,0) << " " << fred(2,1) << " " << fred(2,2) << std::endl;
  std::cout << std::endl;
  /*
  std::cout << "Output d:\n";
  std::cout << d(0,0) << " " << d(0,1) << " " << d(0,2) << std::endl;
  std::cout << d(1,0) << " " << d(1,1) << " " << d(1,2) << std::endl;
  std::cout << d(2,0) << " " << d(2,1) << " " << d(2,2) << std::endl;
  std::cout << std::endl;
  std::cout << "Output rotation:\n";
  std::cout << rot(0,0) << " " << rot(0,1) << " " << rot(0,2) << std::endl;
  std::cout << rot(1,0) << " " << rot(1,1) << " " << rot(1,2) << std::endl;
  std::cout << rot(2,0) << " " << rot(2,1) << " " << rot(2,2) << std::endl;
  std::cout << std::endl;
  std::cout << "Output rotationTrotation:\n";
  rot = rot.transpose() * rot;

  std::cout << rot(0,0) << " " << rot(0,1) << " " << rot(0,2) << std::endl;
  std::cout << rot(1,0) << " " << rot(1,1) << " " << rot(1,2) << std::endl;
  std::cout << rot(2,0) << " " << rot(2,1) << " " << rot(2,2) << std::endl;
  std::cout << std::endl;*/

  exit(0);

}


int
MaterialModel::delta(int i, int j)
{

  if(i == j)
    return 1;
  else
    return 0;
}

////////////////////////////////////////////////////////////////////////

void
MaterialModel::computePreconditioning()
{

  _Jacobian_mult[_qp] = *_elasticity_tensor;


/*
   std::cout << " I am in material model " << std::endl;

   const int ND = 3;

   ColumnMajorMatrix Fdot, I, Finv;
   ColumnMajorMatrix F(_grad_disp_x[_qp], _grad_disp_y[_qp], _grad_disp_z[_qp]);
   ColumnMajorMatrix Fbar(_grad_disp_x_old[_qp], _grad_disp_y_old[_qp], _grad_disp_z_old[_qp]);

   Real term1, term2;

   I.identity();

   F = F.transpose() + I;
   Fbar = Fbar.transpose() + I;

   Fdot = (F - Fbar) * (1.0/ _dt);

   invertMatrix(F,Finv);

   _Jacobian_mult[_qp].zero();

   for(int i = 0; i < ND; i++)
     for(int j = 0; j < ND; j++)
       for(int t = 0; t < ND; t++)
         for(int S = 0; S < ND; S++)
           for(int k = 0; k < ND; k++)
             for(int l = 0; l < ND; l++)
               for(int L = 0; L < ND; L++)
               {
                 term1 = delta(k,t) * delta(L,S) * Finv(L,l) * (1.0/_dt) - Fdot(k,L) * Finv(L,t) * Finv(S,l);
                 term2 = delta(l,t) * delta(L,S) * Finv(L,k) * (1.0/_dt) - Fdot(l,L) * Finv(L,t) * Finv(S,k);

                 _Jacobian_mult[_qp](j*ND+i,S*ND+t) = _Jacobian_mult[_qp](j*ND+i,S*ND+t) + (*_elasticity_tensor)(j*ND+i,l*ND+k) * (term1 + term2) * 0.5;
                 }

                 //_Jacobian_mult[_qp] =  _Jacobian_mult[_qp].transpose();*/


}

void
MaterialModel::subdomainSetup()
{

  if (!_initialized)
  {
    _initialized = true;

    // Load in the volumetric models
    const std::vector<Material*> mats = _moose_system.getMaterials( _tid, _block_id );
    for (unsigned int i(0); i < mats.size(); ++i)
    {
      VolumetricModel * vm(dynamic_cast<VolumetricModel*>(mats[i]));
      if (vm)
      {
        _volumetric_models.push_back( vm );
      }
    }
  }
}
