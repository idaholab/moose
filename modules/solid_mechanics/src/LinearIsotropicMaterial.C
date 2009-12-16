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
  return params;
}

LinearIsotropicMaterial::LinearIsotropicMaterial(std::string name,
                                               InputParameters parameters,
                                               unsigned int block_id,
                                               std::vector<std::string> coupled_to,
                                               std::vector<std::string> coupled_as)
  :SolidMechanicsMaterial(name,parameters,block_id,coupled_to,coupled_as),
   _youngs_modulus(parameters.get<Real>("youngs_modulus")),
   _poissons_ratio(parameters.get<Real>("poissons_ratio"))
{
  IsotropicElasticityTensor * iso_elasticity_tensor = new IsotropicElasticityTensor;
  iso_elasticity_tensor->setYoungsModulus(_youngs_modulus);
  iso_elasticity_tensor->setPoissonsRatio(_poissons_ratio);

  _elasticity_tensor = iso_elasticity_tensor;
}

void
LinearIsotropicMaterial::computeStress(const RealVectorValue & x, const RealVectorValue & y, const RealVectorValue & z, RealTensorValue & stress)
{
  ColumnMajorMatrix strain(x,y,z);

  // 1/2 * (strain + strain^T)
  strain += strain.transpose();
  strain *= 0.5;

  // Create column vector
  strain.reshape(LIBMESH_DIM * LIBMESH_DIM, 1);

  // C * e
  ColumnMajorMatrix stress_vector = (*_elasticity_tensor) * strain;

  // Change 9x1 to a 3x3
  stress_vector.reshape(LIBMESH_DIM, LIBMESH_DIM);

  // Fill the material property
  stress_vector.fill(stress);
}

void
LinearIsotropicMaterial::computeProperties()
{
  for(unsigned int qp=0; qp<_qrule->n_points(); qp++)
  {
    _elasticity_tensor->calculate();

    computeStress(_grad_disp_x[qp], _grad_disp_y[qp], _grad_disp_z[qp], _stress[qp]);
  }
}
