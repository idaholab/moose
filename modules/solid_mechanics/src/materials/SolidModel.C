#include "SolidModel.h"

#include "Problem.h"
#include "SymmIsotropicElasticityTensor.h"
#include "VolumetricModel.h"

template<>
InputParameters validParams<SolidModel>()
{
  //  In order to avoid an uninitialized memory warning, we first addParam
  //    and then set.  We don't want to set an initial value in addParam
  //    since that will also flag the parameter as having had a valid entry
  //    given.
  InputParameters params = validParams<Material>();
  params.addParam<Real>("bulk_modulus", "The bulk modulus for the material.");
  params.set<Real>("bulk_modulus") = -7777;
  params.addParam<Real>("lambda", "Lame's first parameter for the material.");
  params.set<Real>("lambda") = -7777;
  params.addParam<Real>("poissons_ratio", "Poisson's ratio for the material");
  params.set<Real>("poissons_ratio") = -7777;
  params.addParam<Real>("shear_modulus", "The shear modulus of the material.");
  params.set<Real>("shear_modulus") = -7777;
  params.addParam<Real>("youngs_modulus", "Young's modulus of the material.");
  params.set<Real>("youngs_modulus") = -7777;
  params.addParam<std::string>("increment_calculation", "RashidApprox", "The algorithm to use when computing the incremental strain and rotation (RashidApprox or Eigen).");
  params.addParam<Real>("thermal_expansion", 0.0, "The thermal expansion coefficient.");
  params.addCoupledVar("temp", "Coupled Temperature");
  params.addParam<Real>("cracking_strain", "The strain threshold beyond which cracking occurs.  Must be positive.");
  return params;
}



SolidModel::SolidModel( const std::string & name,
                              InputParameters parameters )
  :Material( name, parameters ),
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
   _cracking_strain( parameters.isParamValid("cracking_strain") ?
                     (getParam<Real>("cracking_strain") > 0 ? getParam<Real>("cracking_strain") : -1) : -1 ),
   _has_temp(isCoupled("temp")),
   _temperature(_has_temp ? coupledValue("temp") : _zero),
   _temperature_old(_has_temp ? coupledValueOld("temp") : _zero),
   _alpha(getParam<Real>("thermal_expansion")),
   _volumetric_models(0),
   _stress(declareProperty<SymmTensor>("stress")),
   _stress_old_prop(declarePropertyOld<SymmTensor>("stress")),
   _stress_old(0),
   _total_strain(declareProperty<SymmTensor>("total_strain")),
   _total_strain_old(declarePropertyOld<SymmTensor>("total_strain")),
   _crack_flags(NULL),
   _crack_flags_old(NULL),
   _elasticity_tensor(declareProperty<SymmElasticityTensor>("elasticity_tensor")),
   _Jacobian_mult(declareProperty<SymmElasticityTensor>("Jacobian_mult")),
   _d_strain_dT(),
   _d_stress_dT(declareProperty<SymmTensor>("d_stress_dT")),
   _total_strain_increment(0),
   _strain_increment(0),
   _local_elasticity_tensor(NULL)
{
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

  SymmIsotropicElasticityTensor * iso = new SymmIsotropicElasticityTensor(true);
  iso->setLambda( _lambda );
  iso->setShearModulus( _shear_modulus );
  iso->calculate(0);
  elasticityTensor( iso );

  if (_cracking_strain > 0)
  {
    _crack_flags = &declareProperty<RealVectorValue>("crack_flags");
    _crack_flags_old = &declarePropertyOld<RealVectorValue>("crack_flags");
  }

}

////////////////////////////////////////////////////////////////////////

SolidModel::~SolidModel()
{
  delete _local_elasticity_tensor;
}

////////////////////////////////////////////////////////////////////////

void
SolidModel::elasticityTensor( SymmElasticityTensor * e )
{
  delete _local_elasticity_tensor;
  _local_elasticity_tensor = e;
}

////////////////////////////////////////////////////////////////////////

void
SolidModel::modifyStrainIncrement()
{
  if ( _has_temp )
  {
    const Real tStrain( _alpha * (_temperature[_qp] - _temperature_old[_qp]) );
    _strain_increment.addDiag( -tStrain );

    _d_strain_dT.zero();
    _d_strain_dT.addDiag( -_alpha );
  }

  for (unsigned int i(0); i < _volumetric_models.size(); ++i)
  {
    _volumetric_models[i]->modifyStrain(_qp, _strain_increment, _d_strain_dT);
  }
}

////////////////////////////////////////////////////////////////////////

void
SolidModel::computeStress()
{
  // Given the strain increment, compute the stress increment and add it to the old stress.
  // stress = stressOld + stressIncrement

  SymmTensor stress_new( _elasticity_tensor[_qp] * _strain_increment );
  _stress[_qp] = stress_new;
  _stress[_qp] += _stress_old;

}

////////////////////////////////////////////////////////////////////////

void
SolidModel::rotateSymmetricTensor( const ColumnMajorMatrix & R,
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
SolidModel::computeProperties()
{
  if (_t_step == 1 && _cracking_strain > 0)
  {
    // Initialize crack flags
    for (unsigned int i(0); i < _qrule->n_points(); ++i)
    {
      (*_crack_flags)[i](0) =
        (*_crack_flags)[i](1) =
        (*_crack_flags)[i](2) =
        (*_crack_flags_old)[i](0) =
        (*_crack_flags_old)[i](1) =
        (*_crack_flags_old)[i](2) = 1;
    }
  }

  elementInit();

  for ( _qp = 0; _qp < _qrule->n_points(); ++_qp )
  {
    _local_elasticity_tensor->calculate(_qp);

    _elasticity_tensor[_qp] = *_local_elasticity_tensor;

    computeStrain();

    modifyStrainIncrement();

    crackingStrainRotation();

    computeStress();

    //crackingStressRotation( ... );

    finalizeStress();
    computePreconditioning();

  }
}

////////////////////////////////////////////////////////////////////////

void
SolidModel::fillMatrix( const VariableGradient & grad_x,
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
SolidModel::detMatrix( const ColumnMajorMatrix & A )
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
SolidModel::invertMatrix( const ColumnMajorMatrix & A,
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
SolidModel::computePreconditioning()
{
  mooseAssert(_local_elasticity_tensor, "null elasticity tensor");

  _Jacobian_mult[_qp] = _elasticity_tensor[_qp];

  _d_stress_dT[_qp] = _elasticity_tensor[_qp] * _d_strain_dT;
}

////////////////////////////////////////////////////////////////////////

void
SolidModel::initialSetup()
{
  // Load in the volumetric models
  const std::vector<Material*> & mats = _problem.getMaterials( _block_id, _tid );
  for (unsigned int i(0); i < mats.size(); ++i)
  {
    VolumetricModel * vm(dynamic_cast<VolumetricModel*>(mats[i]));
    if (vm)
    {
      _volumetric_models.push_back( vm );
    }
  }
}

////////////////////////////////////////////////////////////////////////

void
SolidModel::crackingStrainRotation()
{
  _stress_old = _stress_old_prop[_qp];
  bool cracking_locally_active( false );
  RealVectorValue crack_flags;
  ColumnMajorMatrix e_vec(3,3);
  if (_cracking_strain > 0)
  {
    // Compute whether cracking has occurred
    ColumnMajorMatrix principal_strain(3,1);
    _total_strain[_qp].columnMajorMatrix().eigen( principal_strain, e_vec );

    const Real tiny(1e-8);
    for (unsigned int i(0); i < 3; ++i)
    {
      if (principal_strain(i,0) > _cracking_strain)
      {
        (*_crack_flags)[_qp](i) = tiny;
      }
      else
      {
        (*_crack_flags)[_qp](i) = 1;
      }
      (*_crack_flags)[_qp](i) = std::min((*_crack_flags)[_qp](i), (*_crack_flags_old)[_qp](i));
    }
    for (unsigned int i(0); i < 3; ++i)
    {
      if (principal_strain(i,0) < 0)
      {
        crack_flags(i) = 1;
      }
      else
      {
        crack_flags(i) = (*_crack_flags)[_qp](i);
        if (crack_flags(i) < 1)
        {
          cracking_locally_active = true;
        }
      }
    }
  }
  if (cracking_locally_active)
  {
    // Adjust the elasticity matrix for cracking.  This must be used by the
    // constitutive law.
    _elasticity_tensor[_qp] = _local_elasticity_tensor;
    _elasticity_tensor[_qp].adjustForCracking( crack_flags );

    ColumnMajorMatrix R_9x9(9,9);
    _elasticity_tensor[_qp].form9x9Rotation( e_vec, R_9x9 );
    _elasticity_tensor[_qp].rotateFromLocalToGlobal( R_9x9 );

    // Form transformation matrix R*E*R^T
    ColumnMajorMatrix trans(3,3);
    for (unsigned int j(0); j < 3; ++j)
    {
      for (unsigned int i(0); i < 3; ++i)
      {
        for (unsigned int k(0); k < 3; ++k)
        {
          trans(i,j) += e_vec(i,k) * crack_flags(k) * e_vec(j,k);
        }
      }
    }
    // Rotate the old stress to the principal orientation, zero out the stress in
    // cracked directions, and rotate back.  This is done in one step since we have
    // the transformation matrix R*E*R^T.
    rotateSymmetricTensor( trans, _stress_old, _stress_old );
    rotateSymmetricTensor( trans, _strain_increment, _strain_increment );

  }
}
