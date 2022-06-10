//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeStrainBase.h"
#include "RankTwoTensor.h"
#include "SymmetricRankTwoTensor.h"
#include "MooseMesh.h"
#include "Assembly.h"

template <typename R2>
InputParameters
ADComputeStrainBaseTempl<R2>::validParams()
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

template <typename R2>
ADComputeStrainBaseTempl<R2>::ADComputeStrainBaseTempl(const InputParameters & parameters)
  : Material(parameters),
    _ndisp(coupledComponents("displacements")),
    _disp(adCoupledValues("displacements")),
    _grad_disp(adCoupledGradients("displacements")),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _mechanical_strain(declareADProperty<R2>(_base_name + "mechanical_strain")),
    _total_strain(declareADProperty<R2>(_base_name + "total_strain")),
    _eigenstrain_names(getParam<std::vector<MaterialPropertyName>>("eigenstrain_names")),
    _eigenstrains(_eigenstrain_names.size()),
    _global_strain(isParamValid("global_strain")
                       ? &getADMaterialProperty<R2>(_base_name + "global_strain")
                       : nullptr),
    _volumetric_locking_correction(getParam<bool>("volumetric_locking_correction") &&
                                   !this->isBoundaryMaterial()),
    _current_elem_volume(_assembly.elemVolume())
{
  // set unused dimensions to zero
  for (unsigned i = _ndisp; i < 3; ++i)
  {
    _disp.push_back(&_ad_zero);
    _grad_disp.push_back(&_ad_grad_zero);
  }

  for (unsigned int i = 0; i < _eigenstrains.size(); ++i)
  {
    _eigenstrain_names[i] = _base_name + _eigenstrain_names[i];
    _eigenstrains[i] = &getADMaterialProperty<R2>(_eigenstrain_names[i]);
  }

  if (_ndisp == 1 && _volumetric_locking_correction)
    paramError("volumetric_locking_correction", "has to be set to false for 1-D problems.");

  if (getParam<bool>("use_displaced_mesh"))
    paramError("use_displaced_mesh", "The strain calculator needs to run on the undisplaced mesh.");
}

template <typename R2>
void
ADComputeStrainBaseTempl<R2>::initialSetup()
{
  displacementIntegrityCheck();
}

template <typename R2>
void
ADComputeStrainBaseTempl<R2>::displacementIntegrityCheck()
{
  // Checking for consistency between mesh size and length of the provided displacements vector
  if (_ndisp != _mesh.dimension())
    paramError(
        "displacements",
        "The number of variables supplied in 'displacements' must match the mesh dimension.");
}

template <typename R2>
void
ADComputeStrainBaseTempl<R2>::initQpStatefulProperties()
{
  _mechanical_strain[_qp].zero();
  _total_strain[_qp].zero();
}

template class ADComputeStrainBaseTempl<RankTwoTensor>;
template class ADComputeStrainBaseTempl<SymmetricRankTwoTensor>;
