/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ComputeStrainBase.h"
#include "MooseMesh.h"

template<>
InputParameters validParams<ComputeStrainBase>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredCoupledVar("displacements", "The displacements appropriate for the simulation geometry and coordinate system");
  params.addParam<std::string>("base_name", "Optional parameter that allows the user to define multiple mechanics material systems on the same block, i.e. for multiple phases");
  params.addParam<bool>("volumetric_locking_correction", true, "Flag to correct volumetric locking");
  params.addParam<std::vector<MaterialPropertyName>>("eigenstrain_names", "List of eigenstrains to be applied in this strain calculation");
  return params;
}

ComputeStrainBase::ComputeStrainBase(const InputParameters & parameters) :
    DerivativeMaterialInterface<Material>(parameters),
    _ndisp(coupledComponents("displacements")),
    _disp(3),
    _grad_disp(3),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : "" ),
    _mechanical_strain(declareProperty<RankTwoTensor>(_base_name + "mechanical_strain")),
    _total_strain(declareProperty<RankTwoTensor>(_base_name + "total_strain")),
    _eigenstrain_names(getParam<std::vector<MaterialPropertyName>>("eigenstrain_names")),
    _eigenstrains(_eigenstrain_names.size()),
    _volumetric_locking_correction(getParam<bool>("volumetric_locking_correction"))
{
  for (unsigned int i = 0; i < _eigenstrains.size(); ++i)
  {
    _eigenstrain_names[i] = _base_name + _eigenstrain_names[i];
    _eigenstrains[i] = &getMaterialProperty<RankTwoTensor>(_eigenstrain_names[i]);
  }

  // Checking for consistency between mesh size and length of the provided displacements vector
  if (_ndisp != _mesh.dimension())
    mooseError("The number of variables supplied in 'displacements' must match the mesh dimension.");

  // fetch coupled variables and gradients (as stateful properties if necessary)
  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    _disp[i] = &coupledValue("displacements", i);
    _grad_disp[i] = &coupledGradient("displacements", i);
  }

  // set unused dimensions to zero
  for (unsigned i = _ndisp; i < 3; ++i)
  {
    _disp[i] = &_zero;
    _grad_disp[i] = &_grad_zero;
  }
}

void
ComputeStrainBase::initQpStatefulProperties()
{
  _mechanical_strain[_qp].zero();
  _total_strain[_qp].zero();
}
