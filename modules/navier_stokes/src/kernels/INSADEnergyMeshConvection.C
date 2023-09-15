//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADEnergyMeshConvection.h"
#include "INSADMomentumMeshAdvection.h"

registerMooseObject("NavierStokesApp", INSADEnergyMeshConvection);

InputParameters
INSADEnergyMeshConvection::validParams()
{
  InputParameters params = ADKernelValue::validParams();
  params += INSADMomentumMeshAdvection::displacementParams();
  params.addClassDescription("This class computes the residual and Jacobian contributions for "
                             "temperature advection from mesh velocity in an ALE simulation.");
  return params;
}

INSADEnergyMeshConvection::INSADEnergyMeshConvection(const InputParameters & parameters)
  : ADKernelValue(parameters),
    _temperature_convected_mesh_strong_residual(
        getADMaterialProperty<Real>("temperature_convected_mesh_strong_residual"))
{
  INSADMomentumMeshAdvection::setDisplacementParams(*this);
}

ADReal
INSADEnergyMeshConvection::precomputeQpResidual()
{
  return _temperature_convected_mesh_strong_residual[_qp];
}
