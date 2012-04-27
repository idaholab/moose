#include "SolidModel.h"

#include "AxisymmetricRZ.h"
#include "Linear.h"
#include "Nonlinear3D.h"
#include "PlaneStrain.h"

#include "Problem.h"
#include "SymmIsotropicElasticityTensor.h"
#include "SymmIsotropicElasticityTensorRZ.h"
#include "VolumetricModel.h"

template<>
InputParameters validParams<SolidModel>()
{
  InputParameters params = validParams<Material>();
  params.addParam<Real>("bulk_modulus", "The bulk modulus for the material.");
  params.addParam<Real>("lambda", "Lame's first parameter for the material.");
  params.addParam<Real>("poissons_ratio", "Poisson's ratio for the material");
  params.addParam<Real>("shear_modulus", "The shear modulus of the material.");
  params.addParam<Real>("youngs_modulus", "Young's modulus of the material.");
  params.addParam<Real>("thermal_expansion", 0.0, "The thermal expansion coefficient.");
  params.addCoupledVar("temp", "Coupled Temperature");
  params.addParam<Real>("cracking_stress", 0.0, "The stress threshold beyond which cracking occurs.  Must be positive.");
  params.addParam<std::vector<unsigned int> >("active_crack_planes", "Planes on which cracks are allowed (0,1,2 -> x,z,theta in RZ)");
  params.addParam<unsigned int>("max_cracks", 3, "The maximum number of cracks allowed at a material point.");
  params.addParam<std::string>("formulation", "Element formulation.  Choices are \"Nonlinear3D\", \"AxisymmetricRZ\", and \"Linear\".  (Case insensitive.)");
  params.addParam<std::string>("increment_calculation", "RashidApprox", "The algorithm to use when computing the incremental strain and rotation (RashidApprox or Eigen). For use with Nonlinear3D formulation.");
  params.addParam<bool>("large_strain", false, "Whether to include large strain terms in AxisymmetricRZ and PlaneStrain formulations.");
  params.addCoupledVar("disp_r", "The r displacement");
  params.addCoupledVar("disp_x", "The x displacement");
  params.addCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");
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
   _cracking_stress( parameters.isParamValid("cracking_stress") ?
                     (getParam<Real>("cracking_stress") > 0 ? getParam<Real>("cracking_stress") : -1) : -1 ),
   _active_crack_planes(3,1),
   _max_cracks( getParam<unsigned int>("max_cracks") ),
   _has_temp(isCoupled("temp")),
   _temperature(_has_temp ? coupledValue("temp") : _zero),
   _temperature_old(_has_temp ? coupledValueOld("temp") : _zero),
   _alpha(getParam<Real>("thermal_expansion")),
   _stress(declareProperty<SymmTensor>("stress")),
   _stress_old_prop(declarePropertyOld<SymmTensor>("stress")),
   _stress_old(0),
   _total_strain(declareProperty<SymmTensor>("total_strain")),
   _total_strain_old(declarePropertyOld<SymmTensor>("total_strain")),
   _elastic_strain(declareProperty<SymmTensor>("elastic_strain")),
   _elastic_strain_old(declarePropertyOld<SymmTensor>("elastic_strain")),
   _crack_flags(NULL),
   _crack_flags_old(NULL),
   _crack_flags_local(),
   _crack_rotation(NULL),
   _crack_rotation_old(NULL),
   _elasticity_tensor(declareProperty<SymmElasticityTensor>("elasticity_tensor")),
   _Jacobian_mult(declareProperty<SymmElasticityTensor>("Jacobian_mult")),
   _d_strain_dT(),
   _d_stress_dT(declareProperty<SymmTensor>("d_stress_dT")),
   _total_strain_increment(0),
   _strain_increment(0),
   _element(createElement(name, parameters)),
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

  createElasticityTensor();

  if (_cracking_stress > 0)
  {
    _crack_flags = &declareProperty<RealVectorValue>("crack_flags");
    _crack_flags_old = &declarePropertyOld<RealVectorValue>("crack_flags");
    _crack_rotation = &declareProperty<ColumnMajorMatrix>("crack_rotation");
    _crack_rotation_old = &declarePropertyOld<ColumnMajorMatrix>("crack_rotation");

    if (parameters.isParamValid( "active_crack_planes" ))
    {
      const std::vector<unsigned int> & planes = getParam<std::vector<unsigned> >("active_crack_planes");
      for (unsigned i(0); i < 3; ++i)
      {
        _active_crack_planes[i] = 0;
      }
      for (unsigned i(0); i < planes.size(); ++i)
      {
        if (planes[i] > 2)
        {
          mooseError("Active planes must be 0, 1, or 2");
        }
        _active_crack_planes[planes[i]] = 1;
      }
    }
  }

}

////////////////////////////////////////////////////////////////////////

SolidModel::~SolidModel()
{
  delete _local_elasticity_tensor;
  delete _element;
}

////////////////////////////////////////////////////////////////////////

void
SolidModel::createElasticityTensor()
{
  // The IsoRZ and Iso are actually the same...
  if (_coord_sys == Moose::COORD_RZ)
  {
    SymmIsotropicElasticityTensorRZ * t = new SymmIsotropicElasticityTensorRZ;
    mooseAssert(_lambda_set, "Internal error:  lambda not set");
    t->setLambda(_lambda);
    mooseAssert(_shear_modulus_set, "Internal error:  shear modulus not set");
    t->setShearModulus(_shear_modulus);
    t->calculate(0);
    elasticityTensor( t );
  }
  else
  {
    SymmIsotropicElasticityTensor * iso = new SymmIsotropicElasticityTensor(true);
    iso->setLambda( _lambda );
    iso->setShearModulus( _shear_modulus );
    iso->calculate(0);
    elasticityTensor( iso );
  }
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
  SubdomainID current_block = _current_elem->subdomain_id();
  
  if ( _has_temp && _t_step != 0 )
  {
    const Real tStrain( _alpha * (_temperature[_qp] - _temperature_old[_qp]) );
    _strain_increment.addDiag( -tStrain );

    _d_strain_dT.zero();
    _d_strain_dT.addDiag( -_alpha );
  }

  for (unsigned int i(0); i < _volumetric_models[current_block].size(); ++i)
  {
    _volumetric_models[current_block][i]->modifyStrain(_qp, _strain_increment, _d_strain_dT);
  }
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
  if (_t_step == 1 && _cracking_stress > 0)
  {
    // Initialize crack flags, rotation
    for (unsigned int i(0); i < _qrule->n_points(); ++i)
    {
      (*_crack_flags)[i](0) =
        (*_crack_flags)[i](1) =
        (*_crack_flags)[i](2) =
        (*_crack_flags_old)[i](0) =
        (*_crack_flags_old)[i](1) =
        (*_crack_flags_old)[i](2) = 1;

      (*_crack_rotation)[i].identity();
      (*_crack_rotation_old)[i].identity();
    }
  }

  elementInit();
  _element->init();

  for ( _qp = 0; _qp < _qrule->n_points(); ++_qp )
  {
    _local_elasticity_tensor->calculate(_qp);

    _elasticity_tensor[_qp] = *_local_elasticity_tensor;

    _element->computeStrain( _qp,
                             _total_strain_old[_qp],
                             _total_strain[_qp],
                             _strain_increment );
    _total_strain_increment = _strain_increment;

    modifyStrainIncrement();

    crackingStrainDirections();

    computeStress();

    _elastic_strain[_qp] = _elastic_strain_old[_qp] + _strain_increment;

    crackingStressRotation();

    finalizeStress();

    computePreconditioning();

  }
}

////////////////////////////////////////////////////////////////////////

void
SolidModel::finalizeStress()
{
  _element->finalizeStress(_total_strain[_qp],
                           _stress[_qp]);
}

////////////////////////////////////////////////////////////////////////

void
SolidModel::computePreconditioning()
{
  mooseAssert(_local_elasticity_tensor, "null elasticity tensor");

  _Jacobian_mult[_qp] = *_local_elasticity_tensor;
  _d_stress_dT[_qp] = *_local_elasticity_tensor * _d_strain_dT;
}

////////////////////////////////////////////////////////////////////////

void
SolidModel::initialSetup()
{
  // Load in the volumetric models

  for(unsigned int i=0; i<_block_id.size(); i++)
  {
    std::cout<<"Working on block: "<<_block_id[i]<<std::endl;
    
    const std::vector<Material*> * mats_p;
    if(_bnd)
    {
      mats_p = &_problem.getFaceMaterials( _block_id[i], _tid );
    }
    else
    {
      mats_p = &_problem.getMaterials( _block_id[i], _tid );
    }
    
    const std::vector<Material*> & mats = *mats_p;
    for (unsigned int j=0; j < mats.size(); ++j)
    {
      VolumetricModel * vm(dynamic_cast<VolumetricModel*>(mats[j]));
      if (vm)
      {
        std::cout<<"Adding VM! For block: "<<_block_id[i]<<std::endl;
        
        _volumetric_models[_block_id[i]].push_back( vm );
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////

void
SolidModel::crackingStrainDirections()
{
  _stress_old = _stress_old_prop[_qp];

  bool cracking_locally_active( false );
  if (_cracking_stress > 0)
  {
    // Compute whether cracking has occurred
    ColumnMajorMatrix principal_strain(3,1);
    computeCrackStrainAndOrientation( principal_strain );

    for (unsigned int i(0); i < 3; ++i)
    {
      if (principal_strain(i,0) < 0)
      {
        _crack_flags_local(i) = 1;
      }
      else
      {
        _crack_flags_local(i) = (*_crack_flags_old)[_qp](i);
        if (_crack_flags_local(i) < 1)
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
    _elasticity_tensor[_qp].adjustForCracking( _crack_flags_local );

    ColumnMajorMatrix R_9x9(9,9);
    const ColumnMajorMatrix & R( (*_crack_rotation)[_qp] );
    _elasticity_tensor[_qp].form9x9Rotation( R, R_9x9 );
    _elasticity_tensor[_qp].rotateFromLocalToGlobal( R_9x9 );

    // JDH DEBUG:  Perhaps this isn't needed?
//     applyCracksToTensor( _stress_old );

  }
}

////////////////////////////////////////////////////////////////////////

void
SolidModel::applyCracksToTensor( SymmTensor & tensor )
{
  // Form transformation matrix R*E*R^T
  const ColumnMajorMatrix & R( (*_crack_rotation)[_qp] );
  ColumnMajorMatrix trans(3,3);
  for (unsigned int j(0); j < 3; ++j)
  {
    for (unsigned int i(0); i < 3; ++i)
    {
      for (unsigned int k(0); k < 3; ++k)
      {
        trans(i,j) += R(i,k) * _crack_flags_local(k) * R(j,k);
      }
    }
  }
  // Rotate the old stress to the principal orientation, zero out the stress in
  // cracked directions, and rotate back.  This is done in one step since we have
  // the transformation matrix R*E*R^T.
  rotateSymmetricTensor( trans, tensor, tensor );
}

////////////////////////////////////////////////////////////////////////

void
SolidModel::computeCrackStrainAndOrientation( ColumnMajorMatrix & principal_strain )
{
  // The rotation tensor is ordered such that known dirs appear last in the list of
  // columns.  So, if one dir is known, it corresponds with the last column in the
  // rotation tensor.
  //
  // This convention is based on the eigen routine returning eigen values in
  // ascending order.
  const unsigned int numKnownDirs = getNumKnownCrackDirs();

  _elastic_strain[_qp] = _elastic_strain_old[_qp] + _strain_increment;

  (*_crack_rotation)[_qp] = (*_crack_rotation_old)[_qp];

  if (numKnownDirs == 0)
  {
    ColumnMajorMatrix e_vec(3,3);
    _elastic_strain[_qp].columnMajorMatrix().eigen( principal_strain, e_vec );
    // If the elastic strain is beyond the cracking strain, save the eigen vectors as
    // the rotation tensor.
    (*_crack_rotation)[_qp] = e_vec;
  }
  else if (numKnownDirs == 1)
  {
    // This is easily the most complicated case.
    // 1.  Rotate the elastic strain to the orientation associated with the known
    //     crack.
    // 2.  Extract the upper 2x2 diagonal block into a separate tensor.
    // 3.  Run the eigen solver on the result.
    // 4.  Update the rotation tensor to reflect the effect of the 2 eigenvectors.

    // 1.
    ColumnMajorMatrix RT( (*_crack_rotation)[_qp].transpose() );
    SymmTensor ePrime;
    rotateSymmetricTensor( RT, _elastic_strain[_qp], ePrime );

    // 2.
    ColumnMajorMatrix e2x2(2,2);
    e2x2(0,0) = ePrime(0,0);
    e2x2(1,0) = ePrime(1,0);
    e2x2(0,1) = ePrime(0,1);
    e2x2(1,1) = ePrime(1,1);

    // 3.
    ColumnMajorMatrix e_val2x1(2,1);
    ColumnMajorMatrix e_vec2x2(2,2);
    e2x2.eigen( e_val2x1, e_vec2x2 );

    // 4.
    ColumnMajorMatrix e_vec(3,3);
    e_vec(0,0) = e_vec2x2(0,0);
    e_vec(1,0) = e_vec2x2(1,0);
    e_vec(2,0) = 0;
    e_vec(0,1) = e_vec2x2(0,1);
    e_vec(1,1) = e_vec2x2(1,1);
    e_vec(2,1) = 0;
    e_vec(2,0) = 0;
    e_vec(2,1) = 0;
    e_vec(2,2) = 1;
    (*_crack_rotation)[_qp] = (*_crack_rotation_old)[_qp] * e_vec;

    principal_strain(0,0) = e_val2x1(0,0);
    principal_strain(1,0) = e_val2x1(1,0);
    principal_strain(2,0) = ePrime(2,2);

  }
  else if (numKnownDirs == 2 ||
           numKnownDirs == 3)
  {
    // Rotate to cracked orientation and pick off the strains in the rotated
    // coordinate directions.
    ColumnMajorMatrix RT( (*_crack_rotation)[_qp].transpose() );
    SymmTensor ePrime;
    rotateSymmetricTensor( RT, _elastic_strain[_qp], ePrime );
    principal_strain(0,0) = ePrime.xx();
    principal_strain(1,0) = ePrime.yy();
    principal_strain(2,0) = ePrime.zz();
  }
  else
  {
    mooseError("Invalid number of known crack directions");
  }

}

////////////////////////////////////////////////////////////////////////

void
SolidModel::crackingStressRotation()
{

  if (_cracking_stress > 0)
  {

    // Check for new cracks.
    // This must be done in the crack-local coordinate frame.

    // Rotate stress to cracked orientation.
    ColumnMajorMatrix RT( (*_crack_rotation)[_qp].transpose() );
    SymmTensor sigmaPrime;
    rotateSymmetricTensor( RT, _stress[_qp], sigmaPrime );

    bool new_crack(false);
    bool cracked(false);
    const Real tiny(1e-8);
    unsigned int num_cracks(0);
    for (unsigned i(0); i < 3; ++i)
    {
      if ((*_crack_flags_old)[_qp](i) < 1)
      {
        ++num_cracks;
      }
    }
    for (unsigned i(0); i < 3; ++i)
    {
      (*_crack_flags)[_qp](i) = (*_crack_flags_old)[_qp](i);
      if (sigmaPrime(i,i) > _cracking_stress &&
          num_cracks < _max_cracks &&
          _active_crack_planes[i] == 1)
      {
        new_crack = true;
        ++num_cracks;
        (*_crack_flags)[_qp](i) = tiny;
        _crack_flags_local(i) = tiny;

        // Also set the old value.  This helps tremendously with the nonlinear solve
        // since the stress cannot bounce between just below the critical stress and
        // effectively zero.
        (*_crack_flags_old)[_qp](i) = tiny;
      }
      if ((*_crack_flags)[_qp](i) == tiny)
      {
        cracked = true;
        // If cracked in this direction and the stress is positive (tension), we need
        // to ensure that the stress is removed and not rely on _crack_flags_local
        // having the correct value based on the estimate of the elastic strain.
        if (sigmaPrime(i,i) > 0)
        {
          _crack_flags_local(i) = tiny;
        }
      }
    }

    if (!new_crack)
    {
      (*_crack_rotation)[_qp] = (*_crack_rotation_old)[_qp];
    }
    if (cracked)
    {
      applyCracksToTensor( _stress[_qp] );
    }
  }
}

unsigned int
SolidModel::getNumKnownCrackDirs() const
{
  const unsigned fromElement = _element->getNumKnownCrackDirs();
  unsigned int retVal(0);
  for (unsigned int i(0); i < 3-fromElement; ++i)
  {
    retVal += ((*_crack_flags_old)[_qp](i) < 1);
  }
  return retVal+fromElement;
}

Elk::SolidMechanics::Element *
SolidModel::createElement( const std::string & name,
                           InputParameters & parameters )
{
  Elk::SolidMechanics::Element * element(NULL);

  std::string formulation = getParam<std::string>("formulation");
  std::transform( formulation.begin(), formulation.end(),
                  formulation.begin(), ::tolower );
  if ( formulation == "nonlinear3d" )
  {
    if (!isCoupled("disp_x") ||
        !isCoupled("disp_y") ||
        !isCoupled("disp_z"))
    {
      mooseError("Nonlinear3D requires all three displacements");
    }
    if ( isCoupled("disp_r") )
    {
      mooseError("Linear must not define disp_r");
    }
    if ( _coord_sys == Moose::COORD_RZ )
    {
      mooseError("Nonlinear3D formulation requested for coord_type = RZ problem");
    }
    element = new Elk::SolidMechanics::Nonlinear3D(name, parameters);
  }
  else if ( formulation == "axisymmetricrz" )
  {
    if ( !isCoupled("disp_r") ||
         !isCoupled("disp_z") )
    {
      mooseError("AxisymmetricRZ must define disp_r and disp_z");
    }
    element = new Elk::SolidMechanics::AxisymmetricRZ(name, parameters);
  }
  else if ( formulation == "planestrain" )
  {
    if ( !isCoupled("disp_x") ||
         !isCoupled("disp_y") )
    {
      mooseError("PlaneStrain must define disp_x and disp_y");
    }
    element = new Elk::SolidMechanics::PlaneStrain(name, parameters);
  }
  else if ( formulation == "linear" )
  {
    if ( isCoupled("disp_r") )
    {
      mooseError("Linear must not define disp_r");
    }
    if ( _coord_sys == Moose::COORD_RZ )
    {
      mooseError("Linear formulation requested for coord_type = RZ problem");
    }
    element = new Elk::SolidMechanics::Linear(name, parameters);
  }
  else if ( formulation != "" )
  {
    mooseError("Unknown formulation: " + formulation);
  }

  if ( !element && _coord_sys == Moose::COORD_RZ )
  {
    if ( !isCoupled("disp_r") ||
         !isCoupled("disp_z") )
    {
      mooseError("RZ coord sys requires disp_r and disp_z for AxisymmetricRZ formulation");
    }
    element = new Elk::SolidMechanics::AxisymmetricRZ(name, parameters);
  }

  if (!element)
  {
    if (isCoupled("disp_x") &&
        isCoupled("disp_y") &&
        isCoupled("disp_z"))
    {
      if (isCoupled("disp_r"))
      {
        mooseError("Error with displacement specification in material " + name);
      }
      element = new Elk::SolidMechanics::Nonlinear3D(name, parameters);
    }
    else if (isCoupled("disp_r") &&
             isCoupled("disp_z"))
    {
      if ( _coord_sys != Moose::COORD_RZ )
      {
        mooseError("RZ coord system not specified, but disp_r and disp_z are");
      }
      element = new Elk::SolidMechanics::AxisymmetricRZ( name, parameters );
    }
    else if (isCoupled("disp_x"))
    {
      element = new Elk::SolidMechanics::Linear( name, parameters );
    }
    else
    {
      mooseError("Unable to determine formulation for material " + name );
    }

  }

  mooseAssert( element, "No Element created for material " + name );

  return element;
}

