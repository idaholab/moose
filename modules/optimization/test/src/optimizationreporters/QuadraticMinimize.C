#include "QuadraticMinimize.h"

registerMooseObject("isopodTestApp", QuadraticMinimize);

InputParameters
QuadraticMinimize::validParams()
{
  InputParameters params = OptimizationReporter::validParams();
  params.addRequiredParam<Real>("objective", "Desired value of objective function.");
  params.addRequiredParam<std::vector<Real>>("solution", "Desired solution to optimization.");
  return params;
}

QuadraticMinimize::QuadraticMinimize(const InputParameters & parameters)
  : OptimizationReporter(parameters),
    _result(getParam<Real>("objective")),
    _solution(getParam<std::vector<Real>>("solution"))
{
  if (_solution.size() != _ndof)
    paramError("solution", "Size not equal to number of degrees of freedom (", _ndof, ").");
}

Real
QuadraticMinimize::computeAndCheckObjective(bool /*multiapp_passed*/)
{
  Real obj = _result;
  unsigned int i = 0;
  for (const auto & param : _parameters)
    for (const auto & val : *param)
    {
      Real tmp = val - _solution[i++];
      obj += tmp * tmp;
    }

  return obj;
}

void
QuadraticMinimize::computeGradient(libMesh::PetscVector<Number> & gradient)
{
  unsigned int i = 0;
  for (const auto & param : _parameters)
    for (const auto & val : *param)
    {
      gradient.set(i, 2.0 * (val - _solution[i]));
      i++;
    }
  gradient.close();
}
