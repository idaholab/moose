#include "SolidMechanicsMaterialRZ.h"

#include "IsotropicElasticityTensorRZ.h"
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
  params.addRequiredCoupledVar("disp_r", "The r displacement");
  params.addRequiredCoupledVar("disp_z", "The z displacement");
  params.addCoupledVar("temp", "The temperature if you want thermal expansion.");
  return params;
}

SolidMechanicsMaterialRZ::SolidMechanicsMaterialRZ(const std::string & name,
                                                   InputParameters parameters)
  :Material(name, parameters),
   _initialized(false),
   _youngs_modulus(getParam<Real>("youngs_modulus")),
   _poissons_ratio(getParam<Real>("poissons_ratio")),
   _shear_modulus(0.5*_youngs_modulus/(1+_poissons_ratio)),
   _t_ref(getParam<Real>("t_ref")),
   _alpha(getParam<Real>("thermal_expansion")),
   _disp_r(coupledValue("disp_r")),
   _disp_z(coupledValue("disp_z")),
   _grad_disp_r(coupledGradient("disp_r")),
   _grad_disp_z(coupledGradient("disp_z")),
   _has_temp(isCoupled("temp")),
   _temp(_has_temp ? coupledValue("temp") : _zero),
   _volumetric_models(0),
   _stress(declareProperty<RealTensorValue>("stress")),
   _stress_old(declarePropertyOld<RealTensorValue>("stress")),
   _elasticity_tensor(declareProperty<ColumnMajorMatrix>("elasticity_tensor")),
   _Jacobian_mult(declareProperty<ColumnMajorMatrix>("Jacobian_mult")),
   _elastic_strain(declareProperty<ColumnMajorMatrix>("elastic_strain")),
   _v_strain(declareProperty<ColumnMajorMatrix>("v_strain")),
   _v_strain_old(declarePropertyOld<ColumnMajorMatrix>("v_strain")),
   _local_elasticity_tensor(NULL)
{
  IsotropicElasticityTensorRZ * t = new IsotropicElasticityTensorRZ;
  t->setYoungsModulus(_youngs_modulus);
  t->setPoissonsRatio(_poissons_ratio);

  _local_elasticity_tensor = t;
}

SolidMechanicsMaterialRZ::~SolidMechanicsMaterialRZ()
{
  delete _local_elasticity_tensor;
}

void
SolidMechanicsMaterialRZ::subdomainSetup()
{

  if (!_initialized)
  {
    _initialized = true;

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
}

void
SolidMechanicsMaterialRZ::computeProperties()
{
  for(_qp=0; _qp<_n_qpoints; ++_qp)
  {
    _local_elasticity_tensor->calculate(_qp);

    _elasticity_tensor[_qp] = *_local_elasticity_tensor;

    ColumnMajorMatrix strain;
    strain(0,0) = _grad_disp_r[_qp](0);
    strain(1,1) = _grad_disp_z[_qp](1);
    strain(2,2) = _disp_r[_qp]/_q_point[_qp](0);
    strain(0,1) = 0.5*(_grad_disp_r[_qp](1) + _grad_disp_z[_qp](0));
    strain(1,0) = strain(0,1);

    // Add in Isotropic Thermal Strain
    if (_has_temp)
    {
      Real isotropic_strain = _alpha * (_temp[_qp] - _t_ref);

      strain(0,0) -= isotropic_strain;
      strain(1,1) -= isotropic_strain;
      strain(2,2) -= isotropic_strain;
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

    computeStress(strain, *_local_elasticity_tensor, _stress[_qp]);

  }
}

void
SolidMechanicsMaterialRZ::computeStress(const ColumnMajorMatrix & strain,
                                        const ElasticityTensor & elasticity_tensor,
                                        RealTensorValue & stress)
{
  // Add in any extra strain components
  ColumnMajorMatrix elastic_strain;

  computeStrain(strain, elastic_strain);

  // Save that off as the elastic strain
  _elastic_strain[_qp] = elastic_strain;


  // Create column vector
  elastic_strain.reshape(LIBMESH_DIM * LIBMESH_DIM, 1);

  // C * e
  ColumnMajorMatrix stress_vector = elasticity_tensor * elastic_strain;

  // Change 9x1 to a 3x3
  stress_vector.reshape(LIBMESH_DIM, LIBMESH_DIM);

  stress_vector += _stress_old[_qp];

  // Fill the material properties
  stress_vector.fill(stress);
}
