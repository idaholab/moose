//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADMeshConvection.h"
#include "INSADObjectTracker.h"

registerMooseObject("NavierStokesApp", INSADMeshConvection);

InputParameters
INSADMeshConvection::displacementParams()
{
  auto params = emptyInputParameters();
  params.addRequiredCoupledVar("disp_x", "The x displacement");
  params.addCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");
  return params;
}

InputParameters
INSADMeshConvection::validParams()
{
  InputParameters params = ADVectorKernelValue::validParams();
  params += INSADMeshConvection::displacementParams();
  params.addClassDescription(
      "Corrects the convective derivative for situations in which the fluid mesh is dynamic.");
  return params;
}

INSADMeshConvection::INSADMeshConvection(const InputParameters & parameters)
  : ADVectorKernelValue(parameters),
    _convected_mesh_strong_residual(
        getADMaterialProperty<RealVectorValue>("convected_mesh_strong_residual"))
{
  setDisplacementParams(*this);
}

ADRealVectorValue
INSADMeshConvection::precomputeQpResidual()
{
  return _convected_mesh_strong_residual[_qp];
}
