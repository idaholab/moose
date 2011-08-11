#include "SolidMechanicsMaterialRZ.h"

#include "SymmIsotropicElasticityTensorRZ.h"
#include "MaterialModel.h"
#include "Problem.h"
#include "VolumetricModel.h"

template<>
InputParameters validParams<SolidMechanicsMaterialRZ>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<Real>("youngs_modulus", "Young's Modulus");
  params.addRequiredParam<Real>("poissons_ratio", "Poisson's Ratio");
  params.addParam<Real>("thermal_expansion", 0.0, "The thermal expansion coefficient.");
  params.addParam<bool>("large_strain", false, "Whether to include large strain terms");
  params.addRequiredCoupledVar("disp_r", "The r displacement");
  params.addRequiredCoupledVar("disp_z", "The z displacement");
  params.addCoupledVar("temp", "The temperature if you want thermal expansion.");
  params.addParam<Real>("cracking_strain", "The strain threshold beyond which cracking occurs.  Must be positive.");
  return params;
}

SolidMechanicsMaterialRZ::SolidMechanicsMaterialRZ(const std::string & name,
                                                   InputParameters parameters)
  :Material(name, parameters),
   _youngs_modulus(getParam<Real>("youngs_modulus")),
   _poissons_ratio(getParam<Real>("poissons_ratio")),
   _shear_modulus(0.5*_youngs_modulus/(1+_poissons_ratio)),
   _large_strain(getParam<bool>("large_strain")),
   _cracking_strain( parameters.isParamValid("cracking_strain") ?
                     (getParam<Real>("cracking_strain") > 0 ? getParam<Real>("cracking_strain") : -1) : -1 ),
   _cracking_locally_active(false),
   _e_vec(3,3),
   _alpha(getParam<Real>("thermal_expansion")),
   _disp_r(coupledValue("disp_r")),
   _disp_z(coupledValue("disp_z")),
   _grad_disp_r(coupledGradient("disp_r")),
   _grad_disp_z(coupledGradient("disp_z")),
   _has_temp(isCoupled("temp")),
   _temp(_has_temp ? coupledValue("temp") : _zero),
   _temp_old(_has_temp ? coupledValueOld("temp") : _zero),
   _volumetric_models(0),
   _stress(declareProperty<SymmTensor>("stress")),
   _stress_old(declarePropertyOld<SymmTensor>("stress")),
   _total_strain(declareProperty<SymmTensor>("total_strain")),
   _total_strain_old(declarePropertyOld<SymmTensor>("total_strain")),
   _crack_flags(NULL),
   _crack_flags_old(NULL),
   _elasticity_tensor(declareProperty<SymmElasticityTensor>("elasticity_tensor")),
   _Jacobian_mult(declareProperty<SymmElasticityTensor>("Jacobian_mult")),
   _d_strain_dT(),
   _d_stress_dT(declareProperty<SymmTensor>("d_stress_dT")),
   _elastic_strain(declareProperty<SymmTensor>("elastic_strain")),
   _stress_old_temp(),
   _local_elasticity_tensor(NULL)
{
  SymmIsotropicElasticityTensorRZ * t = new SymmIsotropicElasticityTensorRZ;
  t->setYoungsModulus(_youngs_modulus);
  t->setPoissonsRatio(_poissons_ratio);

  _local_elasticity_tensor = t;

  if (_cracking_strain > 0)
  {
    _crack_flags = &declareProperty<RealVectorValue>("crack_flags");
    _crack_flags_old = &declarePropertyOld<RealVectorValue>("crack_flags");
  }

}

SolidMechanicsMaterialRZ::~SolidMechanicsMaterialRZ()
{
  delete _local_elasticity_tensor;
}

void
SolidMechanicsMaterialRZ::initialSetup()
{
  // Load in the volumetric models
  const std::vector<Material*> * mats_p;
  if(_bnd)
  {
    mats_p = &_problem.getFaceMaterials( _block_id, _tid );
  }
  else
  {
    mats_p = &_problem.getMaterials( _block_id, _tid );
  }

  const std::vector<Material*> & mats = *mats_p;

  for (unsigned int i(0); i < mats.size(); ++i)
  {
    VolumetricModel * vm(dynamic_cast<VolumetricModel*>(mats[i]));
    if (vm)
    {
      _volumetric_models.push_back( vm );
    }
  }
}

void
SolidMechanicsMaterialRZ::computeProperties()
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

  for(_qp=0; _qp < _qrule->n_points(); ++_qp)
  {
    _local_elasticity_tensor->calculate(_qp);

    _elasticity_tensor[_qp] = *_local_elasticity_tensor;

    SymmTensor strain;
    strain.xx() = _grad_disp_r[_qp](0);
    strain.yy() = _grad_disp_z[_qp](1);
    strain.zz() = _disp_r[_qp]/_q_point[_qp](0);
    strain.xy() = 0.5*(_grad_disp_r[_qp](1) + _grad_disp_z[_qp](0));
    if (_large_strain)
    {
      strain.xx() += 0.5*(_grad_disp_r[_qp](0)*_grad_disp_r[_qp](0) +
                          _grad_disp_z[_qp](0)*_grad_disp_z[_qp](0));
      strain.yy() += 0.5*(_grad_disp_r[_qp](1)*_grad_disp_r[_qp](1) +
                          _grad_disp_z[_qp](1)*_grad_disp_z[_qp](1));
      strain.zz() += 0.5*(strain.zz()*strain.zz());
      strain.xy() += 0.5*(_grad_disp_r[_qp](0)*_grad_disp_r[_qp](1) +
                          _grad_disp_z[_qp](0)*_grad_disp_z[_qp](1));
    }

    _total_strain[_qp] = strain;

    strain -= _total_strain_old[_qp];

    // Add in Isotropic Thermal Strain Increment
    if (_has_temp)
    {
      Real isotropic_strain = _alpha * (_temp[_qp] - _temp_old[_qp]);

      strain.addDiag( -isotropic_strain );

      _d_strain_dT.zero();
      _d_strain_dT.addDiag( -_alpha );
    }

    const unsigned int num_vol_models(_volumetric_models.size());
    if (num_vol_models)
    {
      SymmTensor dv_strain_dT(0);
      for (unsigned int i(0); i < num_vol_models; ++i)
      {
        _volumetric_models[i]->modifyStrain(_qp, strain, dv_strain_dT);
      }

      dv_strain_dT *= _dt;
      _d_strain_dT += dv_strain_dT;
    }

    computeStress(_total_strain[_qp], _elasticity_tensor[_qp], strain, _stress[_qp]);

    computePreconditioning();

  }
}

void
SolidMechanicsMaterialRZ::computeStress(const SymmTensor & total_strain,
                                        const SymmElasticityTensor & elasticity_tensor,
                                        SymmTensor & strain,
                                        SymmTensor & stress)
{
  // Add in any extra strain components
  SymmTensor elastic_strain;

  crackingStrainRotation( total_strain, strain );

  computeStrain(strain, elastic_strain);

  // Save that off as the elastic strain
  _elastic_strain[_qp] = elastic_strain;

  // C * e
  stress = elasticity_tensor * elastic_strain;
  stress += _stress_old_temp;

  crackingStressRotation();

//   computeCracking( total_strain, stress );
}

void
SolidMechanicsMaterialRZ::computePreconditioning()
{
  //Jacobian multiplier of the stress
  mooseAssert(_local_elasticity_tensor, "null _local_elasticity_tensor");
  _Jacobian_mult[_qp] = _elasticity_tensor[_qp];
  SymmTensor d_stress_dT( _elasticity_tensor[_qp] * _d_strain_dT );
//   d_stress_dT *= _dt;
  _d_stress_dT[_qp] = d_stress_dT;
}

void
SolidMechanicsMaterialRZ::crackingStrainRotation( const SymmTensor & total_strain,
                                                  SymmTensor & strain_increment )
{
  _stress_old_temp = _stress_old[_qp];
  _cracking_locally_active = false;
  RealVectorValue crack_flags;
  if (_cracking_strain > 0)
  {
    // Compute whether cracking has occurred
    ColumnMajorMatrix principal_strain(3,1);
    total_strain.columnMajorMatrix().eigen( principal_strain, _e_vec );

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
          _cracking_locally_active = true;
        }
      }
    }
  }
  if (_cracking_locally_active)
  {
    // Adjust the elasticity matrix for cracking.  This must be used by the
    // constitutive law.
    _elasticity_tensor[_qp] = *_local_elasticity_tensor;
    _elasticity_tensor[_qp].adjustForCracking( crack_flags );

    ColumnMajorMatrix R_9x9(9,9);
    _elasticity_tensor[_qp].form9x9Rotation( _e_vec, R_9x9 );
    _elasticity_tensor[_qp].rotateFromLocalToGlobal( R_9x9 );

    // Form transformation matrix R*E*R^T
    ColumnMajorMatrix trans(3,3);
    for (unsigned int j(0); j < 3; ++j)
    {
      for (unsigned int i(0); i < 3; ++i)
      {
        for (unsigned int k(0); k < 3; ++k)
        {
          trans(i,j) += _e_vec(i,k) * crack_flags(k) * _e_vec(j,k);
        }
      }
    }
    // Rotate the old stress to the principal orientation, zero out the stress in
    // cracked directions, and rotate back.  This is done in one step since we have
    // the transformation matrix R*E*R^T.
    MaterialModel::rotateSymmetricTensor( trans, _stress_old_temp, _stress_old_temp );
    MaterialModel::rotateSymmetricTensor( trans, strain_increment, strain_increment );

  }
}

void
SolidMechanicsMaterialRZ::crackingStressRotation()
{
  // If everything is already performed in the global frame, there is no need to
  // rotate to global.
//   if ( _cracking_locally_active )
//   {
//     MaterialModel::rotateSymmetricTensor( _e_vec, _stress_old[_qp], _stress_old[_qp] );
//     MaterialModel::rotateSymmetricTensor( _e_vec, _stress[_qp], _stress[_qp] );
//   }
}

void
SolidMechanicsMaterialRZ::computeCracking( const SymmTensor & strain,
                                           SymmTensor & stress )
{
  if (_cracking_strain > 0)
  {
    // Adjust stress for a smeared crack
    ColumnMajorMatrix e_vec(3,3);
    ColumnMajorMatrix principal_strain(3,1);
    strain.columnMajorMatrix().eigen( principal_strain, e_vec );

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
    RealVectorValue crack_flags( (*_crack_flags)[_qp] );
    for (unsigned int i(0); i < 3; ++i)
    {
      if (principal_strain(i,0) < 0)
      {
        crack_flags(i) = 1;
      }
      else
      {
        crack_flags(i) = (*_crack_flags)[_qp](i);
      }
    }
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
    MaterialModel::rotateSymmetricTensor( trans, stress, stress );
  }
}
