#include "FormFunction.h"

InputParameters
FormFunction::validParams()
{
  InputParameters params = MooseObject::validParams();

  params.addRequiredParam<std::vector<Real>>("initial_condition",
                                             "Initial parameters values for optimization.");

  params.registerBase("FormFunction");
  params.registerSystemAttributeName("FormFunction");
  return params;
}

FormFunction::FormFunction(const InputParameters & parameters)
  : MooseObject(parameters),
    _initial_condition(getParam<std::vector<Real>>("initial_condition")),
    _ndof(_parameters.size()),
    _parameters(_communicator, _ndof, SERIAL),
    _gradient(_communicator, _ndof, SERIAL),
    _hessian(_communicator)
{
  _parameters = _initial_condition;
  _hessian.init(/*global_rows =*/_ndof,
                /*global_cols =*/_ndof,
                /*local_rows =*/_ndof,
                /*local_cols =*/_ndof,
                /*block_diag_nz =*/_ndof,
                /*block_off_diag_nz =*/0);
}
