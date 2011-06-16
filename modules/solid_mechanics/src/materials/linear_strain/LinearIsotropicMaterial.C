#include "LinearIsotropicMaterial.h"

// Elk Includes
#include "ColumnMajorMatrix.h"
#include "SolidMechanicsMaterial.h"
#include "SymmIsotropicElasticityTensor.h"
#include "VolumetricModel.h"

template<>
InputParameters validParams<LinearIsotropicMaterial>()
{
  InputParameters params = validParams<SolidMechanicsMaterial>();
  params.addRequiredParam<Real>("youngs_modulus", "Young's Modulus");
  params.addRequiredParam<Real>("poissons_ratio", "Poisson's Ratio");
  params.addParam<Real>("t_ref", 0.0, "The reference temperature at which this material has zero strain.");
  params.addParam<Real>("thermal_expansion", 0.0, "The thermal expansion coefficient.");
  params.addParam<Real>("thermal_conductivity", 0.0, "The thermal conductivity coeffecient.");
  return params;
}

LinearIsotropicMaterial::LinearIsotropicMaterial(const std::string  & name,
                                                 InputParameters parameters)
  :SolidMechanicsMaterial(name, parameters),
   _youngs_modulus(getParam<Real>("youngs_modulus")),
   _poissons_ratio(getParam<Real>("poissons_ratio")),
   _t_ref(getParam<Real>("t_ref")),
   _alpha(getParam<Real>("thermal_expansion")),
   _input_thermal_conductivity(getParam<Real>("thermal_conductivity")),
   _local_elasticity_tensor(NULL)
{
  SymmIsotropicElasticityTensor * iso_elasticity_tensor = new SymmIsotropicElasticityTensor;
  iso_elasticity_tensor->setYoungsModulus(_youngs_modulus);
  iso_elasticity_tensor->setPoissonsRatio(_poissons_ratio);

  _local_elasticity_tensor = iso_elasticity_tensor;
}

LinearIsotropicMaterial::~LinearIsotropicMaterial()
{
  delete _local_elasticity_tensor;
}

void
LinearIsotropicMaterial::computeProperties()
{
  for(_qp=0; _qp<_qrule->n_points(); _qp++)
  {
    _thermal_conductivity[_qp] = _input_thermal_conductivity;

    _local_elasticity_tensor->calculate(_qp);

    _elasticity_tensor[_qp] = *_local_elasticity_tensor;


    SymmTensor strn( _grad_disp_x[_qp](0),
                     _grad_disp_y[_qp](1),
                     _grad_disp_z[_qp](2),
                     0.5*(_grad_disp_x[_qp](1)+_grad_disp_y[_qp](0)),
                     0.5*(_grad_disp_y[_qp](2)+_grad_disp_z[_qp](1)),
                     0.5*(_grad_disp_z[_qp](0)+_grad_disp_x[_qp](2)) );

    // Add in Isotropic Thermal Strain
    if(_has_temp)
    {
      Real isotropic_strain = _alpha * (_temp[_qp] - _t_ref);

      strn.addDiag( -isotropic_strain );
    }

    SymmTensor v_strain(0);
    for (unsigned int i(0); i < _volumetric_models.size(); ++i)
    {
      _volumetric_models[i]->modifyStrain(_qp, v_strain);
    }
    SymmTensor strain( v_strain );
    strain *= _dt;
    strain += strn;

    computeStress(strain, _stress[_qp]);

  }
}

void
LinearIsotropicMaterial::computeStress(const SymmTensor & strain,
                                       SymmTensor & stress)
{
  // Add in any extra strain components
  SymmTensor elastic_strain;

  computeStrain(strain, elastic_strain);

  // Save that off as the elastic strain
  _elastic_strain[_qp] = elastic_strain;

  // Create column vector
  // C * e
  stress = (*_local_elasticity_tensor) * elastic_strain;

}

void
LinearIsotropicMaterial::computeStrain(const SymmTensor & total_strain, SymmTensor & elastic_strain)
{
  elastic_strain = total_strain;
  //Jacobian multiplier of the stress
  _Jacobian_mult[_qp] = *_local_elasticity_tensor;
}
