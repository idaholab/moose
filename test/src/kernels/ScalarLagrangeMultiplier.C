#include "ScalarLagrangeMultiplier.h"

template<>
InputParameters validParams<ScalarLagrangeMultiplier>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar("lambda", "Lagrange multiplier variable");

  return params;
}

ScalarLagrangeMultiplier::ScalarLagrangeMultiplier(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _lambda_var(coupledScalar("lambda")),
    _lambda(coupledScalarValue("lambda"))
{
}

ScalarLagrangeMultiplier::~ScalarLagrangeMultiplier()
{
}

Real
ScalarLagrangeMultiplier::computeQpResidual()
{
  return _lambda[0] * _test[_i][_qp];
}

void
ScalarLagrangeMultiplier::computeOffDiagJacobianScalar(unsigned int jvar)
{
  DenseMatrix<Number> & ken = _assembly.jacobianBlock(_var.index(), jvar);
  DenseMatrix<Number> & kne = _assembly.jacobianBlock(jvar, _var.index());
  MooseVariableScalar & jv = _sys.getScalarVariable(_tid, jvar);

  for (_i = 0; _i < _test.size(); _i++)
    for (_j = 0; _j < jv.order(); _j++)
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      {
        Real value = _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobian(jvar);
        ken(_i, _j) += value;
        kne(_j, _i) += value;
      }
}

Real
ScalarLagrangeMultiplier::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _lambda_var)
    return _test[_i][_qp];
  else
    return 0.;
}
