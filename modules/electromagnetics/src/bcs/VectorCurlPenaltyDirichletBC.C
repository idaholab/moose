#include "VectorCurlPenaltyDirichletBC.h"
#include "Function.h"

registerMooseObject("ElkApp", VectorCurlPenaltyDirichletBC);

/**
 * Based on VectorCurlPenaltyDirichletBC in 'tests' directories in core MOOSE Framework.
 */

InputParameters
VectorCurlPenaltyDirichletBC::validParams()
{
  InputParameters params = VectorIntegratedBC::validParams();
  params.addClassDescription("Dirichlet boundary condition using a penalty method to set the value "
                             "of a vector variable on a boundary.");
  params.addRequiredParam<Real>("penalty", "The penalty coefficient.");
  params.addParam<FunctionName>("x_exact_soln", 0, "The exact solution for the x component.");
  params.addParam<FunctionName>("y_exact_soln", 0, "The exact solution for the y component.");
  params.addParam<FunctionName>("z_exact_soln", 0, "The exact solution for the z component.");
  return params;
}

VectorCurlPenaltyDirichletBC::VectorCurlPenaltyDirichletBC(const InputParameters & parameters)
  : VectorIntegratedBC(parameters),
    _penalty(getParam<Real>("penalty")),
    _exact_x(getFunction("x_exact_soln")),
    _exact_y(getFunction("y_exact_soln")),
    _exact_z(getFunction("z_exact_soln"))
{
}

Real
VectorCurlPenaltyDirichletBC::computeQpResidual()
{
  RealVectorValue u_exact = {_exact_x.value(_t, _q_point[_qp]),
                             _exact_y.value(_t, _q_point[_qp]),
                             _exact_z.value(_t, _q_point[_qp])};
  RealVectorValue u_cross_n = (_u[_qp] - u_exact).cross(_normals[_qp]);
  return _penalty * u_cross_n * ((_test[_i][_qp]).cross(_normals[_qp]));
}

Real
VectorCurlPenaltyDirichletBC::computeQpJacobian()
{
  return _penalty * ((_phi[_j][_qp]).cross(_normals[_qp])) *
         ((_test[_i][_qp]).cross(_normals[_qp]));
}
