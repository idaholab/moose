#include "ObjectiveGradientMinimize.h"

registerMooseObject("isopodApp", ObjectiveGradientMinimize);

InputParameters
ObjectiveGradientMinimize::validParams()
{
  InputParameters params = FormFunction::validParams();
  params.addParam<ReporterName>(
      "misfit_computed",
      "Name of reporter value containing difference between measured and calculated point values.");
  params.addParam<ReporterValueName>(
      "misfit_name", "Reporter value to create if 'adjoint_data_computed' does not exist.");

  params.addParam<ReporterName>("adjoint_data_computed",
                                "Name of reporter value containing adjoint point values.");
  params.addParam<ReporterValueName>(
      "adjoint_data_name", "Reporter value to create if 'adjoint_data_computed' does not exist.");
  return params;
}

ObjectiveGradientMinimize::ObjectiveGradientMinimize(const InputParameters & parameters)
  : FormFunction(parameters),
    _misfit(getDataValueHelper("misfit_computed", "misfit_name")),
    _adjoint_data(getDataValueHelper("adjoint_data_computed", "adjoint_data_name"))
{
}

Real
ObjectiveGradientMinimize::computeObjective()
{
  Real val = 0;
  for (auto & misfit : _misfit)
    val += misfit * misfit;

  val = 0.5 * val;

  return val;
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
