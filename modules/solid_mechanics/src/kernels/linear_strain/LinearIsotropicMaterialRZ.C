#include "LinearIsotropicMaterialRZ.h"

// Elk Includes
#include "ColumnMajorMatrix.h"
#include "IsotropicElasticityTensorRZ.h"
#include "SolidMechanicsMaterial.h"

template<>
InputParameters validParams<LinearIsotropicMaterialRZ>()
{
  InputParameters params = validParams<SolidMechanicsMaterialRZ>();
  params.addRequiredParam<Real>("youngs_modulus", "Young's Modulus");
  params.addRequiredParam<Real>("poissons_ratio", "Poisson's Ratio");
  params.addParam<Real>("t_ref", 0.0, "The reference temperature at which this material has zero strain.");
  params.addParam<Real>("thermal_expansion", 0.0, "The thermal expansion coefficient.");
  params.addParam<Real>("thermal_conductivity", 0.0, "The thermal conductivity coeffecient.");
  return params;
}

LinearIsotropicMaterialRZ::LinearIsotropicMaterialRZ(const std::string  & name,
                                                                         InputParameters parameters)
  :SolidMechanicsMaterialRZ(name, parameters),
   _youngs_modulus(getParam<Real>("youngs_modulus")),
   _poissons_ratio(getParam<Real>("poissons_ratio")),
   _t_ref(getParam<Real>("t_ref")),
   _alpha(getParam<Real>("thermal_expansion")),
   _input_thermal_conductivity(getParam<Real>("thermal_conductivity"))
{
  IsotropicElasticityTensorRZ * t = new IsotropicElasticityTensorRZ;
  t->setYoungsModulus(_youngs_modulus);
  t->setPoissonsRatio(_poissons_ratio);

  _local_elasticity_tensor = t;
}

LinearIsotropicMaterialRZ::~LinearIsotropicMaterialRZ()
{
  delete _local_elasticity_tensor;
}

void
LinearIsotropicMaterialRZ::computeProperties()
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

    computeStress(strain, _stress[_qp]);

    if (_qp == 0)
    {
//       std::cout << "JDH DEBUG: strain, stress:\n";
//       std::cout << _grad_disp_r[_qp](0) << " " << _grad_disp_z[_qp](0) << std::endl;
//       std::cout << _grad_disp_r[_qp](1) << " " << _grad_disp_z[_qp](1) << std::endl;
//       strain.print();
//       _stress[_qp].print();
    }
  }
}

void
LinearIsotropicMaterialRZ::computeStress(const ColumnMajorMatrix & strain, RealTensorValue & stress)
{

  ColumnMajorMatrix elastic_strain;

  computeStrain(strain, elastic_strain);

  // Save that off as the elastic strain
  _elastic_strain[_qp] = elastic_strain;

  // Add in Isotropic Thermal Strain
  if(_has_temp)
  {
    Real isotropic_strain = _alpha * (_temp[_qp] - _t_ref);

    elastic_strain(0,0) -= isotropic_strain;
    elastic_strain(1,1) -= isotropic_strain;
    elastic_strain(2,2) -= isotropic_strain;
  }

  // Create column vector
  elastic_strain.reshape(LIBMESH_DIM * LIBMESH_DIM, 1);

  // C * e
  ColumnMajorMatrix stress_vector = (*_local_elasticity_tensor) * elastic_strain;

  // Change 9x1 to a 3x3
  stress_vector.reshape(LIBMESH_DIM, LIBMESH_DIM);

  // Fill the material properties
  stress_vector.fill(stress);

}

void
LinearIsotropicMaterialRZ::computeStrain(const ColumnMajorMatrix & total_strain, ColumnMajorMatrix & elastic_strain)
{
  elastic_strain = total_strain;
  //Jacobian multiplier of the stress
  _Jacobian_mult[_qp] = *_local_elasticity_tensor;
}

