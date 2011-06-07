#include "SolidMechanicsMaterialRZ.h"

#include "IsotropicElasticityTensorRZ.h"
#include "MaterialModel.h"
#include "Problem.h"
#include "VolumetricModel.h"

template<>
InputParameters validParams<SolidMechanicsMaterialRZ>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<Real>("youngs_modulus", "Young's Modulus");
  params.addRequiredParam<Real>("poissons_ratio", "Poisson's Ratio");
  params.addParam<Real>("t_ref", 0.0, "The reference temperature at which this material has zero strain.");
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
   _t_ref(getParam<Real>("t_ref")),
   _alpha(getParam<Real>("thermal_expansion")),
   _disp_r(coupledValue("disp_r")),
   _disp_z(coupledValue("disp_z")),
   _grad_disp_r(coupledGradient("disp_r")),
   _grad_disp_z(coupledGradient("disp_z")),
   _has_temp(isCoupled("temp")),
   _temp(_has_temp ? coupledValue("temp") : _zero),
   _volumetric_models(0),
   _stress(declareProperty<SymmTensor>("stress")),
   _stress_old(declarePropertyOld<SymmTensor>("stress")),
   _crack_flags(NULL),
   _crack_flags_old(NULL),
   _elasticity_tensor(declareProperty<ColumnMajorMatrix>("elasticity_tensor")),
   _Jacobian_mult(declareProperty<ColumnMajorMatrix>("Jacobian_mult")),
   _elastic_strain(declareProperty<SymmTensor>("elastic_strain")),
   _v_strain(declareProperty<SymmTensor>("v_strain")),
   _v_strain_old(declarePropertyOld<SymmTensor>("v_strain")),
   _local_elasticity_tensor(NULL)
{
  IsotropicElasticityTensorRZ * t = new IsotropicElasticityTensorRZ;
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

    ColumnMajorMatrix tot_strain;
    tot_strain(0,0) = _grad_disp_r[_qp](0);
    tot_strain(1,1) = _grad_disp_z[_qp](1);
    tot_strain(2,2) = _disp_r[_qp]/_q_point[_qp](0);
    tot_strain(0,1) = 0.5*(_grad_disp_r[_qp](1) + _grad_disp_z[_qp](0));
    if (_large_strain)
    {
      tot_strain(0,0) += 0.5*(_grad_disp_r[_qp](0)*_grad_disp_r[_qp](0) +
                              _grad_disp_z[_qp](0)*_grad_disp_z[_qp](0));
      tot_strain(1,1) += 0.5*(_grad_disp_r[_qp](1)*_grad_disp_r[_qp](1) +
                              _grad_disp_z[_qp](1)*_grad_disp_z[_qp](1));
      tot_strain(2,2) += 0.5*(tot_strain(2,2)*tot_strain(2,2));
      tot_strain(0,1) += 0.5*(_grad_disp_r[_qp](0)*_grad_disp_r[_qp](1) +
                              _grad_disp_z[_qp](0)*_grad_disp_z[_qp](1));
    }
    tot_strain(1,0) = tot_strain(0,1);

    SymmTensor total_strain;
    total_strain = tot_strain;
    SymmTensor strain( total_strain );

    // Add in Isotropic Thermal Strain
    if (_has_temp)
    {
      Real isotropic_strain = _alpha * (_temp[_qp] - _t_ref);

      strain.addDiag( -isotropic_strain );
    }

    const unsigned int num_vol_models(_volumetric_models.size());
    if (num_vol_models)
    {
      _v_strain[_qp].zero();
      for (unsigned int i(0); i < _volumetric_models.size(); ++i)
      {
        _volumetric_models[i]->modifyStrain(_qp, _v_strain[_qp]);
      }
      _v_strain[_qp] *= _dt;
      _v_strain[_qp] += _v_strain_old[_qp];
      strain += _v_strain[_qp];
    }

    computeStress(total_strain,
                  strain, *_local_elasticity_tensor, _stress[_qp]);

  }
}

void
SolidMechanicsMaterialRZ::computeStress(const SymmTensor & total_strain,
                                        const SymmTensor & strain,
                                        const ElasticityTensor & elasticity_tensor,
                                        SymmTensor & stress)
{
  // Add in any extra strain components
  SymmTensor elastic_strain;

  computeStrain(strain, elastic_strain);

  // Save that off as the elastic strain
  _elastic_strain[_qp] = elastic_strain;

  // C * e
  ColumnMajorMatrix el_strn( elastic_strain.columnMajorMatrix() );
  el_strn.reshape( 9, 1 );
  ColumnMajorMatrix stress_vector = elasticity_tensor * el_strn;

  // Fill the material properties
  stress = stress_vector;
  stress += _stress_old[_qp];

  computeCracking( total_strain, stress );
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
