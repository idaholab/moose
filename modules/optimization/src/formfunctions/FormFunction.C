#include "FormFunction.h"

InputParameters
FormFunction::validParams()
{
  InputParameters params = MooseObject::validParams();

  params.addRequiredParam<std::vector<Real>>("initial_condition",
                                             "Initial parameters values for optimization.");
  params.addRequiredParam<VectorPostprocessorName>(
      "optimization_vpp", "OptimizationVectorPostprocessor vector postprocessor.");
  params.addRequiredParam<VectorPostprocessorName>("optimization_results",
                                                   "OptimizationResults vector postprocessor.");
  params.registerBase("FormFunction");
  params.registerSystemAttributeName("FormFunction");
  return params;
}

FormFunction::FormFunction(const InputParameters & parameters)
  : MooseObject(parameters),
    _my_comm(MPI_COMM_SELF),
    _initial_condition(getParam<std::vector<Real>>("initial_condition")),
    _results_vpp(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")
                     ->getUserObject<OptimizationVectorPostprocessor>(
                         getParam<VectorPostprocessorName>("optimization_vpp"))),
    _results_vpp2(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")
                      ->getUserObject<OptimizationResults>(
                          getParam<VectorPostprocessorName>("optimization_results"))),
    _ndof(_initial_condition.size()),
    _parameters(_my_comm, _ndof),
    _gradient(_my_comm, _ndof),
    _hessian(_my_comm)
{
  _results_vpp.setParameterValues(_initial_condition);
  //_results_vpp2.setParameterValues(_initial_condition);
  _parameters = _initial_condition;
  _hessian.init(/*global_rows =*/_ndof,
                /*global_cols =*/_ndof,
                /*local_rows =*/_ndof,
                /*local_cols =*/_ndof,
                /*block_diag_nz =*/_ndof,
                /*block_off_diag_nz =*/0);
}

void
FormFunction::setParameters(const libMesh::PetscVector<Number> & x)
{
  _parameters = x;

  std::vector<Real> transfer;
  _parameters.localize(transfer);
  _results_vpp.setParameterValues(transfer);
  _results_vpp2.setParameterValues(transfer);
}
