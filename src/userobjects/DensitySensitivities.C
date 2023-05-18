#include "DensitySensitivities.h"
#include "MooseError.h"
#include <algorithm>

registerMooseObject("troutApp", DensitySensitivities);

InputParameters
DensitySensitivities::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addRequiredParam<UserObjectName>("filter_UO", "Radial Average user object");
  params.addRequiredCoupledVar("density_sensitivity", "Name of the density_sensitivity variable.");
  params.addRequiredParam<VariableName>("design_density", "Design density variable name.");

  return params;
}

DensitySensitivities::DensitySensitivities(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _filter(getUserObject<RadialAverage>("filter_UO").getAverage()),
    _density_sensitivity(writableVariable("density_sensitivity")),
    _design_density_name(getParam<VariableName>("design_density")),
    _design_density(_subproblem.getStandardVariable(_tid, _design_density_name))
{
}

void
DensitySensitivities::execute()
{
  // Find the current element in the filter
  auto filter_iter = _filter.find(_current_elem->id());

  // If the element is not found in the filter, throw an error
  if (filter_iter == _filter.end())
  {
    mooseError("Could not find the element in the filter.");
  }

  // Get the quadrature point values from the filter
  std::vector<Real> qp_vals = filter_iter->second;

  // Initialize the total elemental sensitivity value
  Real den_sense_val = 0;

  // Compute the total elemental sensitivity value by summing over all quadrature points
  for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
  {
    den_sense_val += qp_vals[qp] * _JxW[qp];
  }
  den_sense_val /= _current_elem_volume;
  // Normalize the total elemental sensitivity value by the design density and the volume of the
  // current element This makes the sensitivity mesh size independent A small value (1e-3) is
  // used to avoid division by zero
  den_sense_val /= std::max(1e-3, _design_density.getElementalValue(_current_elem));

  // Set the nodal value of the density sensitivity
  _density_sensitivity.setNodalValue(den_sense_val);
}
