#include "FormFunction.h"
#include "VectorPostprocessorPointSource.h"

InputParameters
FormFunction::validParams()
{
  InputParameters params = MooseObject::validParams();

  params.addRequiredParam<VectorPostprocessorName>(
      "optimization_vpp",
      "OptimizationVectorPostprocessor used for transferring data between simulation and "
      "optimizer.");
  params.addRequiredParam<VectorPostprocessorName>("measurement_vpp",
                                                   "Vector of measured solution data.");
  params.registerBase("FormFunction");
  params.registerSystemAttributeName("FormFunction");
  return params;
}

FormFunction::FormFunction(const InputParameters & parameters)
  : MooseObject(parameters),
    VectorPostprocessorInterface(this),
    _my_comm(MPI_COMM_SELF),
    _results_vpp(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")
                     ->getUserObject<OptimizationVectorPostprocessor>(
                         getParam<VectorPostprocessorName>("optimization_vpp"))),
    _measurement_vpp_values(getVectorPostprocessorValue(
        "measurement_vpp", "values", false)), // fixme lynn values should not be hardcoded
    _parameters(_my_comm),
    _gradient(_my_comm),
    _hessian(_my_comm)
{
}

void
FormFunction::initializePetscVectors()
{
  _ndof = _results_vpp.getNumberOfParameters();
  _parameters.init(_ndof);
  _parameters = _results_vpp.getParameterValues();

  _gradient.init(_ndof);
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
}
