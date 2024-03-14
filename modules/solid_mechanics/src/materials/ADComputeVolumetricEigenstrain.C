//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeVolumetricEigenstrain.h"
#include "RankTwoTensor.h"

registerMooseObject("SolidMechanicsApp", ADComputeVolumetricEigenstrain);

InputParameters
ADComputeVolumetricEigenstrain::validParams()
{
  InputParameters params = ADComputeEigenstrainBase::validParams();
  params.addClassDescription("Computes an eigenstrain that is defined by a set of scalar material "
                             "properties that summed together define the volumetric change.");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "volumetric_materials", "List of scalar material properties defining the volumetric change");
  return params;
}

ADComputeVolumetricEigenstrain::ADComputeVolumetricEigenstrain(const InputParameters & parameters)
  : ADComputeEigenstrainBase(parameters),
    _volumetric_materials(
        getParam<std::vector<MaterialPropertyName>>("volumetric_materials").size())
{
  const auto volumetric_material_names =
      getParam<std::vector<MaterialPropertyName>>("volumetric_materials");
  for (unsigned int i = 0; i < volumetric_material_names.size(); ++i)
    _volumetric_materials[i] = &getADMaterialProperty<Real>(volumetric_material_names[i]);
}

void
ADComputeVolumetricEigenstrain::computeQpEigenstrain()
{
  ADReal volumetric_strain = 0.0;
  for (unsigned int i = 0; i < _volumetric_materials.size(); ++i)
    volumetric_strain += (*_volumetric_materials[i])[_qp];

  const auto eigenstrain_comp = computeVolumetricStrainComponent(volumetric_strain);
  _eigenstrain[_qp].zero();
  _eigenstrain[_qp].addIa(eigenstrain_comp);
}
