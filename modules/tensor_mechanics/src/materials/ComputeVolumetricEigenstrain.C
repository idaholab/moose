//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeVolumetricEigenstrain.h"
#include "RankTwoTensor.h"

registerMooseObject("TensorMechanicsApp", ComputeVolumetricEigenstrain);

InputParameters
ComputeVolumetricEigenstrain::validParams()
{
  InputParameters params = ComputeEigenstrainBase::validParams();
  params.addClassDescription("Computes an eigenstrain that is defined by a set of scalar material "
                             "properties that summed together define the volumetric change.  This "
                             "also computes the derivatives of that eigenstrain with respect to a "
                             "supplied set of variable dependencies.");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "volumetric_materials", "List of scalar material properties defining the volumetric change");
  params.addRequiredCoupledVar("args", "variable dependencies for the volumetric_expansion");
  return params;
}

ComputeVolumetricEigenstrain::ComputeVolumetricEigenstrain(const InputParameters & parameters)
  : DerivativeMaterialInterface<ComputeEigenstrainBase>(parameters),
    _num_args(coupledComponents("args")),
    _volumetric_material_names(getParam<std::vector<MaterialPropertyName>>("volumetric_materials")),
    _volumetric_materials(_volumetric_material_names.size()),
    _dvolumetric_materials(_volumetric_material_names.size()),
    _d2volumetric_materials(_volumetric_material_names.size()),
    _delastic_strain(_num_args),
    _d2elastic_strain(_num_args)
{
  for (unsigned int i = 0; i < _volumetric_material_names.size(); ++i)
    _volumetric_materials[i] = &getMaterialProperty<Real>(_volumetric_material_names[i]);

  // fetch prerequisite derivatives and build elastic_strain derivatives and cross-derivatives
  for (unsigned int i = 0; i < _volumetric_material_names.size(); ++i)
  {
    const MaterialPropertyName & vol_matl_name = _volumetric_material_names[i];
    _dvolumetric_materials[i].resize(_num_args);
    _d2volumetric_materials[i].resize(_num_args);
    for (unsigned int j = 0; j < _num_args; ++j)
    {
      const VariableName & jname = coupledName("args", j);
      _dvolumetric_materials[i][j] = &getMaterialPropertyDerivative<Real>(vol_matl_name, jname);
      _d2volumetric_materials[i][j].resize(_num_args);

      for (unsigned int k = j; k < _num_args; ++k)
      {
        const VariableName & kname = coupledName("args", k);
        _d2volumetric_materials[i][j][k] =
            &getMaterialPropertyDerivative<Real>("prefactor", jname, kname);
      }
    }
  }

  for (unsigned int j = 0; j < _num_args; ++j)
  {
    const VariableName & jname = coupledName("args", j);
    _delastic_strain[j] =
        &declarePropertyDerivative<RankTwoTensor>(_base_name + "elastic_strain", jname);
    _d2elastic_strain[j].resize(_num_args);

    for (unsigned int k = j; k < _num_args; ++k)
    {
      const VariableName & kname = coupledName("args", k);
      _d2elastic_strain[j][k] =
          &declarePropertyDerivative<RankTwoTensor>(_base_name + "elastic_strain", jname, kname);
    }
  }
}

void
ComputeVolumetricEigenstrain::initialSetup()
{
  for (auto vmn : _volumetric_material_names)
    validateCoupling<Real>(vmn);

  for (unsigned int i = 0; i < _num_args; ++i)
  {
    const VariableName & iname = coupledName("args", i);
    if (_fe_problem.isMatPropRequested(
            derivativePropertyNameFirst(_base_name + "elastic_strain", iname)))
      mooseError("Derivative of elastic_strain requested, but not yet implemented");
    else
      _delastic_strain[i] = nullptr;
    for (unsigned int j = 0; j < _num_args; ++j)
    {
      const VariableName & jname = coupledName("args", j);
      if (_fe_problem.isMatPropRequested(
              derivativePropertyNameSecond(_base_name + "elastic_strain", iname, jname)))
        mooseError("Second Derivative of elastic_strain requested, but not yet implemented");
      else
        _d2elastic_strain[i][j] = nullptr;
    }
  }
}

void
ComputeVolumetricEigenstrain::computeQpEigenstrain()
{
  Real volumetric_strain = 0;
  for (unsigned int i = 0; i < _volumetric_materials.size(); ++i)
    volumetric_strain += (*_volumetric_materials[i])[_qp];

  const Real eigenstrain_comp = computeVolumetricStrainComponent(volumetric_strain);
  _eigenstrain[_qp].zero();
  _eigenstrain[_qp].addIa(eigenstrain_comp);

  // TODO: Compute derivatives of the elastic strain wrt the variables specified in args
}
