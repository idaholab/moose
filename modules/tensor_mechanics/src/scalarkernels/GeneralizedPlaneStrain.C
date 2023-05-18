//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneralizedPlaneStrain.h"

// MOOSE includes
#include "Assembly.h"
#include "GeneralizedPlaneStrainUOInterface.h"
#include "MooseVariableScalar.h"
#include "SystemBase.h"
#include "ReferenceResidualProblem.h"
#include "InputParameters.h"

#include <algorithm>

registerMooseObject("TensorMechanicsApp", GeneralizedPlaneStrain);

InputParameters
GeneralizedPlaneStrain::validParams()
{
  InputParameters params = ScalarKernel::validParams();
  params.addClassDescription("Generalized Plane Strain Scalar Kernel");
  params.addRequiredParam<UserObjectName>("generalized_plane_strain",
                                          "The name of the GeneralizedPlaneStrainUserObject");
  params.addParam<unsigned int>(
      "scalar_out_of_plane_strain_index",
      "The index number of scalar_out_of_plane_strain this kernel acts on");

  return params;
}

GeneralizedPlaneStrain::GeneralizedPlaneStrain(const InputParameters & parameters)
  : ScalarKernel(parameters),
    _gps(getUserObject<GeneralizedPlaneStrainUOInterface>("generalized_plane_strain")),
    _scalar_var_id(isParamValid("scalar_out_of_plane_strain_index")
                       ? getParam<unsigned int>("scalar_out_of_plane_strain_index")
                       : 0)
{
}

void
GeneralizedPlaneStrain::computeResidual()
{
  prepareVectorTag(_assembly, _var.number(), TaggingInterface::ResidualTagType::NonReference);
  for (_i = 0; _i < _local_re.size(); ++_i)
    _local_re(_i) += _gps.returnResidual(_scalar_var_id);
  accumulateTaggedLocalResidual();

  prepareVectorTag(_assembly, _var.number(), TaggingInterface::ResidualTagType::Reference);
  for (_i = 0; _i < _local_re.size(); ++_i)
    _local_re(_i) += _gps.returnReferenceResidual(_scalar_var_id);
  accumulateTaggedLocalResidual();
}

/**
 * method to provide the diagonal jacobian term for scalar variable using value
 * returned from Postprocessor, off diagonal terms are computed by computeOffDiagJacobianScalar
 * in the kernel of nonlinear variables which needs to couple with the scalar variable
 */
void
GeneralizedPlaneStrain::computeJacobian()
{
  prepareMatrixTag(_assembly, _var.number(), _var.number());
  for (_i = 0; _i < _local_ke.m(); ++_i)
    _local_ke(_i, _i) += _gps.returnJacobian(_scalar_var_id);

  accumulateTaggedLocalMatrix();
}
