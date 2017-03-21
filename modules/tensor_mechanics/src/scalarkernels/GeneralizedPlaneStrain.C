/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "GeneralizedPlaneStrain.h"
#include "Assembly.h"
#include "GeneralizedPlaneStrainUserObject.h"

template <>
InputParameters
validParams<GeneralizedPlaneStrain>()
{
  InputParameters params = validParams<ScalarKernel>();
  params.addClassDescription("Generalized Plane Strain Scalar Kernel");
  params.addRequiredParam<UserObjectName>("generalized_plane_strain",
                                          "The name of the GeneralizedPlaneStrainUO UserObject");

  return params;
}

GeneralizedPlaneStrain::GeneralizedPlaneStrain(const InputParameters & parameters)
  : ScalarKernel(parameters),
    _gps(getUserObject<GeneralizedPlaneStrainUserObject>("generalized_plane_strain"))
{
}

void
GeneralizedPlaneStrain::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());
  for (_i = 0; _i < re.size(); ++_i)
    re(_i) += _gps.returnResidual();
}

/**
 * method to provide the diagonal jacobian term for scalar variable using value
 * returned from Postprocessor, off diagonal terms are computed by computeOffDiagJacobianScalar
 * in the kernel of nonlinear variables which needs to couple with the scalar variable
 */
void
GeneralizedPlaneStrain::computeJacobian()
{
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), _var.number());
  for (_i = 0; _i < ke.m(); ++_i)
    ke(_i, _i) += _gps.returnJacobian();
}
