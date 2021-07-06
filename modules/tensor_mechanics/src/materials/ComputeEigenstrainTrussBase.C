//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeEigenstrainTrussBase.h"

InputParameters
ComputeEigenstrainTrussBase::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredParam<std::string>("eigenstrain_name",
                                       "Material property name for the eigenstrain vector computed "
                                       "by this model. IMPORTANT: The name of this property must "
                                       "also be provided to the strain calculator.");
  return params;
}

ComputeEigenstrainTrussBase::ComputeEigenstrainTrussBase(const InputParameters & parameters)
  : Material(parameters),
    _eigenstrain_name(getParam<std::string>("eigenstrain_name")),
    _disp_eigenstrain(declareProperty<RealVectorValue>("disp_" + _eigenstrain_name)),
    _step_zero(declareRestartableData<bool>("step_zero", true))
{
}

void
ComputeEigenstrainTrussBase::initQpStatefulProperties()
{
  // This property can be promoted to be stateful by other models that use it,
  // so it needs to be initalized.
  _disp_eigenstrain[_qp] = RealVectorValue();
}

void
ComputeEigenstrainTrussBase::computeQpProperties()
{
  if (_t_step >= 1)
    _step_zero = false;

  // Skip the eigenstrain calculation in step zero because no solution is computed during
  // the zeroth step, hence computing the eigenstrain in the zeroth step would result in
  // an incorrect calculation of mechanical_strain, which is stateful.
  if (!_step_zero)
    computeQpEigenstrain();
}
