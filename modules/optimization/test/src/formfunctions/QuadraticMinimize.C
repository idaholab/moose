#include "QuadraticMinimize.h"

registerMooseObject("isopodApp", QuadraticMinimize);

InputParameters
QuadraticMinimize::validParams()
{
  InputParameters params = FormFunction::validParams();
  params.addRequiredParam<Real>("objective", "Desired value of objective function.");
  params.addRequiredParam<VectorPostprocessorName>("measured_vpp",
                                                   "VectorPostprocessor of measured data.");
  return params;
}

QuadraticMinimize::QuadraticMinimize(const InputParameters & parameters)
  : FormFunction(parameters),
    _result(getParam<Real>("objective")),
    _measured_values(getVectorPostprocessorValue(
        "measured_vpp", "values", false)) // fixme lynn values should not be hardcoded
{
}

Real
QuadraticMinimize::computeObjective()
{
  Real val = _result;
  for (dof_id_type i = 0; i < _ndof; ++i)
  {
    Real tmp = (*_parameters[i])[0] - _measured_values[i];
    val += tmp * tmp;
  }

  return val;
}

void
QuadraticMinimize::computeGradient(libMesh::PetscVector<Number> & gradient)
{
  for (dof_id_type i = 0; i < _ndof; ++i)
    gradient.set(i, 2.0 * ((*_parameters[i])[0] - _measured_values[i]));
  gradient.close();
}

void
QuadraticMinimize::computeHessian(libMesh::PetscMatrix<Number> & hessian)
{
  hessian.zero();
  for (dof_id_type i = 0; i < _ndof; ++i)
    hessian.set(i, i, 2.0);
  hessian.close();
}
