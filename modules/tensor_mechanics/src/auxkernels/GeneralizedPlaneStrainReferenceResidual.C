//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneralizedPlaneStrainReferenceResidual.h"

// MOOSE includes
#include "Assembly.h"
#include "GeneralizedPlaneStrainUserObject.h"
#include "MooseVariableScalar.h"
#include "SystemBase.h"

registerMooseObject("TensorMechanicsApp", GeneralizedPlaneStrainReferenceResidual);

InputParameters
GeneralizedPlaneStrainReferenceResidual::validParams()
{
  InputParameters params = AuxScalarKernel::validParams();
  params.addClassDescription("Generalized Plane Strain Reference Residual Scalar Kernel");
  params.addRequiredParam<UserObjectName>("generalized_plane_strain",
                                          "The name of the GeneralizedPlaneStrainUserObject");
  params.addParam<unsigned int>(
      "scalar_out_of_plane_strain_index",
      "The index number of scalar_out_of_plane_strain this kernel acts on");

  return params;
}

GeneralizedPlaneStrainReferenceResidual::GeneralizedPlaneStrainReferenceResidual(
    const InputParameters & parameters)
  : AuxScalarKernel(parameters),
    _gps(getUserObject<GeneralizedPlaneStrainUserObject>("generalized_plane_strain")),
    _scalar_var_id(isParamValid("scalar_out_of_plane_strain_index")
                       ? getParam<unsigned int>("scalar_out_of_plane_strain_index")
                       : 0)
{
}

Real
GeneralizedPlaneStrainReferenceResidual::computeValue()
{
  return _gps.returnReferenceResidual(_scalar_var_id);
}
