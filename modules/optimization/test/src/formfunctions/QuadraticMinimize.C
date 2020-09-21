#include "QuadraticMinimize.h"

registerMooseObject("isopodApp", QuadraticMinimize);

InputParameters
QuadraticMinimize::validParams()
{
  InputParameters params = FormFunction::validParams();
  params.addRequiredParam<Real>("objective", "Desired value of objective function.");
  params.addRequiredParam<std::vector<Real>>("solution", "Desired solution of minimization.");
  return params;
}

QuadraticMinimize::QuadraticMinimize(const InputParameters & parameters)
  : FormFunction(parameters),
    _result(getParam<Real>("objective")),
    _solution(getParam<std::vector<Real>>("solution"))
{
}

Real
QuadraticMinimize::computeObjective()
{
  Real val = _result;
  for (dof_id_type i = 0; i < _ndof; ++i)
  {
    Real tmp = _parameters(i) - _solution[i];
    val += tmp * tmp;
  }

  return val;
}

void
QuadraticMinimize::computeGradient()
{
  for (dof_id_type i = 0; i < _ndof; ++i)
    _gradient.set(i, 2.0 * (_parameters(i) - _solution[i]));
  _gradient.close();
}

void
QuadraticMinimize::computeHessian()
{
  _hessian.zero();
  for (dof_id_type i = 0; i < _ndof; ++i)
    _hessian.set(i, i, 2.0);
  _hessian.close();
}
