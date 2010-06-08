#include "LinearIsotropicMaterial.h"

// Elk Includes
#include "ColumnMajorMatrix.h"
#include "IsotropicElasticityTensor.h"
#include "SolidMechanicsMaterial.h"

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

LinearIsotropicMaterial::LinearIsotropicMaterial(std::string name,
                                                 MooseSystem & moose_system,
                                                 InputParameters parameters)
  :SolidMechanicsMaterial(name, moose_system, parameters),
   _youngs_modulus(parameters.get<Real>("youngs_modulus")),
   _poissons_ratio(parameters.get<Real>("poissons_ratio")),
   _t_ref(parameters.get<Real>("t_ref")),
   _alpha(parameters.get<Real>("thermal_expansion")),
   _input_thermal_conductivity(parameters.get<Real>("thermal_conductivity"))
{
  IsotropicElasticityTensor * iso_elasticity_tensor = new IsotropicElasticityTensor;
  iso_elasticity_tensor->setYoungsModulus(_youngs_modulus);
  iso_elasticity_tensor->setPoissonsRatio(_poissons_ratio);

  _local_elasticity_tensor = iso_elasticity_tensor;
}

void
LinearIsotropicMaterial::computeStress(const RealVectorValue & x, const RealVectorValue & y, const RealVectorValue & z, RealTensorValue & stress)
{
  ColumnMajorMatrix strain(x,y,z);

  // 1/2 * (strain + strain^T)
  strain += strain.transpose();
  strain *= 0.5;

  // Save that off as the elastic strain
  _elastic_strain[_qp] = strain;

  // Add in any extra strain components
  computeStrain(strain);

  // Add in Isotropic Thermal Strain
  if(_has_temp)
  {
    ColumnMajorMatrix thermal_strain;

    Real isotropic_strain = _alpha * (_temp[_qp] - _t_ref);
    
    thermal_strain.setDiag(isotropic_strain);
    
    strain -= thermal_strain;
  }

  // Create column vector
  strain.reshape(LIBMESH_DIM * LIBMESH_DIM, 1);

  // C * e
  ColumnMajorMatrix stress_vector = (*_local_elasticity_tensor) * strain;

  // Change 9x1 to a 3x3
  stress_vector.reshape(LIBMESH_DIM, LIBMESH_DIM);

  // Fill the material property
  stress_vector.fill(stress);
}

void
LinearIsotropicMaterial::computeStrain(ColumnMajorMatrix & strain)
{}

void
LinearIsotropicMaterial::computeProperties()
{
  for(_qp=0; _qp<_qrule->n_points(); _qp++)
  {
    _thermal_conductivity[_qp] = _input_thermal_conductivity;
    
    _local_elasticity_tensor->calculate();

    _elasticity_tensor[_qp] = *_local_elasticity_tensor;

    computeStress(_grad_disp_x[_qp], _grad_disp_y[_qp], _grad_disp_z[_qp], _stress[_qp]);
  }
}
