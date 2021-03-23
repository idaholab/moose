#include "ObjectiveGradientMinimize.h"

registerMooseObject("isopodApp", ObjectiveGradientMinimize);

InputParameters
ObjectiveGradientMinimize::validParams()
{
  InputParameters params = FormFunction::validParams();

  params.addParam<ReporterName>("adjoint_data_computed",
                                "Name of reporter value containing adjoint point values.");
  params.addParam<ReporterValueName>(
      "adjoint_data_name", "Reporter value to create if 'adjoint_data_computed' does not exist.");
  return params;
}

ObjectiveGradientMinimize::ObjectiveGradientMinimize(const InputParameters & parameters)
  : FormFunction(parameters),
    _adjoint_data(getDataValueHelper("adjoint_data_computed", "adjoint_data_name"))
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
