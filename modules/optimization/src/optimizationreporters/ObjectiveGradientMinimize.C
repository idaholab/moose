#include "ObjectiveGradientMinimize.h"

registerMooseObject("isopodApp", ObjectiveGradientMinimize);

InputParameters
ObjectiveGradientMinimize::validParams()
{
  InputParameters params = OptimizationReporter::validParams();
  params.addClassDescription("OptimizationReporter that holds optimization information and "
                             "computes gradient from adjoint data.");
  return params;
}

ObjectiveGradientMinimize::ObjectiveGradientMinimize(const InputParameters & parameters)
  : OptimizationReporter(parameters),
    _adjoint_data(declareValueByName<std::vector<Real>>("adjoint", REPORTER_MODE_REPLICATED))
{
}

void
ObjectiveGradientMinimize::computeGradient(libMesh::PetscVector<Number> & gradient)
{
  if (_adjoint_data.size() != _ndof)
    mooseError("Adjoint data is not equal to the total number of parameters.");

  for (dof_id_type i = 0; i < _ndof; ++i)
    gradient.set(i, _adjoint_data[i]);
  gradient.close();
}
