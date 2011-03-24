#include "LinearIsotropicMaterialRZ.h"

// Elk Includes
#include "ElasticityTensor.h"
#include "SolidMechanicsMaterial.h"
#include "VolumetricModel.h"

template<>
InputParameters validParams<LinearIsotropicMaterialRZ>()
{
  InputParameters params = validParams<SolidMechanicsMaterialRZ>();
  params.addParam<Real>("thermal_conductivity", 0.0, "The thermal conductivity coeffecient.");
  return params;
}

LinearIsotropicMaterialRZ::LinearIsotropicMaterialRZ(const std::string  & name,
                                                     InputParameters parameters)
  :SolidMechanicsMaterialRZ(name, parameters),
   _input_thermal_conductivity(getParam<Real>("thermal_conductivity"))
{
}

LinearIsotropicMaterialRZ::~LinearIsotropicMaterialRZ()
{
}

void
LinearIsotropicMaterialRZ::computeStress(const ColumnMajorMatrix & total_strain,
                                         const ColumnMajorMatrix & strain,
                                         const ElasticityTensor & elasticity_tensor,
                                         RealTensorValue & stress)
{

  ColumnMajorMatrix elastic_strain;

  computeStrain(strain, elastic_strain);

  // Save that off as the elastic strain
  _elastic_strain[_qp] = elastic_strain;

  // Create column vector
  elastic_strain.reshape(LIBMESH_DIM * LIBMESH_DIM, 1);

  // C * e
  ColumnMajorMatrix stress_vector( elasticity_tensor * elastic_strain );

  // Change 9x1 to a 3x3
  stress_vector.reshape(LIBMESH_DIM, LIBMESH_DIM);

  // Fill the material properties
  stress_vector.fill(stress);

  computeCracking( total_strain, stress );

}

void
LinearIsotropicMaterialRZ::computeStrain(const ColumnMajorMatrix & total_strain, ColumnMajorMatrix & elastic_strain)
{
  elastic_strain = total_strain;
  //Jacobian multiplier of the stress
  _Jacobian_mult[_qp] = *_local_elasticity_tensor;
}

