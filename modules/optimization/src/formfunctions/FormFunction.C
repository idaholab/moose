#include "FormFunction.h"

InputParameters
FormFunction::validParams()
{
  InputParameters params = MooseObject::validParams();

  params.addRequiredParam<VectorPostprocessorName>(
      "parameter_vpp",
      "OptimizationParameterVpp used for transferring parameters between simulation and "
      "optimizer.");
  params.registerBase("FormFunction");
  params.registerSystemAttributeName("FormFunction");
  return params;
}

FormFunction::FormFunction(const InputParameters & parameters)
  : MooseObject(parameters),
    VectorPostprocessorInterface(this),
    PostprocessorInterface(this),
    _parameter_vpp(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")
                       ->getUserObject<OptimizationParameterVectorPostprocessor>(
                           getParam<VectorPostprocessorName>("parameter_vpp")))
{
}

void
FormFunction::setInitialCondition(libMesh::PetscVector<Number> & x)
{
  _ndof = _parameter_vpp.getNumberOfParameters();
  _parameters.reserve(_ndof);
  for (const auto & nm : _parameter_vpp.getParameterNames())
    _parameters.push_back(&getVectorPostprocessorValue("parameter_vpp", nm));

  x.init(_ndof);
  x = _parameter_vpp.getParameterValues();
}

void
FormFunction::updateParameters(const libMesh::PetscVector<Number> & x)
{
  std::vector<Real> transfer;
  x.localize(transfer);
  _parameter_vpp.setParameterValues(transfer);
}
