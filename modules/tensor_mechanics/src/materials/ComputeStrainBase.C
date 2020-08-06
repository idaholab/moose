//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeStrainBase.h"
#include "MooseMesh.h"
#include "Assembly.h"

InputParameters
ComputeStrainBase::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredCoupledVar(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
  params.addParam<bool>(
      "volumetric_locking_correction", false, "Flag to correct volumetric locking");
  params.addParam<std::vector<MaterialPropertyName>>(
      "eigenstrain_names", "List of eigenstrains to be applied in this strain calculation");
  params.addParam<MaterialPropertyName>("global_strain",
                                        "Optional material property holding a global strain "
                                        "tensor applied to the mesh as a whole");
  params.suppressParameter<bool>("use_displaced_mesh");
  return params;
}

ComputeStrainBase::ComputeStrainBase(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _ndisp(coupledComponents("displacements")),
    _disp(coupledValues("displacements")),
    _grad_disp(coupledGradients("displacements")),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _mechanical_strain(declareProperty<RankTwoTensor>(_base_name + "mechanical_strain")),
    _total_strain(declareProperty<RankTwoTensor>(_base_name + "total_strain")),
    _eigenstrain_names(getParam<std::vector<MaterialPropertyName>>("eigenstrain_names")),
    _eigenstrains(_eigenstrain_names.size()),
    _global_strain(isParamValid("global_strain")
                       ? &getMaterialProperty<RankTwoTensor>(_base_name + "global_strain")
                       : nullptr),
    _volumetric_locking_correction(getParam<bool>("volumetric_locking_correction") &&
                                   !isBoundaryMaterial()),
    _current_elem_volume(_assembly.elemVolume())
{
  for (unsigned int i = 0; i < _eigenstrains.size(); ++i)
  {
    _eigenstrain_names[i] = _base_name + _eigenstrain_names[i];
    _eigenstrains[i] = &getMaterialProperty<RankTwoTensor>(_eigenstrain_names[i]);
  }

  // set unused dimensions to zero
  _disp.resize(3, &_zero);
  _grad_disp.resize(3, &_grad_zero);

  if (_ndisp == 1 && _volumetric_locking_correction)
    paramError("volumetric_locking_correction", "has to be set to false for 1-D problems.");

  if (getParam<bool>("use_displaced_mesh"))
    paramError("use_displaced_mesh", "The strain calculator needs to run on the undisplaced mesh.");

  // Generate warning when volumetric locking correction is used with second order elements
  if (_mesh.hasSecondOrderElements() && _volumetric_locking_correction)
    mooseWarning("Volumetric locking correction is not required for second order elements. Using "
                 "volumetric locking with second order elements could cause zigzag patterns in "
                 "stresses and strains.");
}

void
ComputeStrainBase::initialSetup()
{
  displacementIntegrityCheck();
}

void
ComputeStrainBase::displacementIntegrityCheck()
{
  // Checking for consistency between mesh size and length of the provided displacements vector
  if (_ndisp != _mesh.dimension())
    paramError(
        "displacements",
        "The number of variables supplied in 'displacements' must match the mesh dimension.");
}

void
ComputeStrainBase::initQpStatefulProperties()
{
  _mechanical_strain[_qp].zero();
  _total_strain[_qp].zero();
}
