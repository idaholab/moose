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
  auto filter_iter = _filter.find(_current_elem->id());

  if (filter_iter == _filter.end())
  {
    mooseError("Could not find the element in the filter.");
  }
  std::vector<Real> qp_vals = filter_iter->second;
  Real den_sense_val = 0;
  for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
  {
    den_sense_val += qp_vals[qp] * _JxW[qp];
  }
  // for this part of the code
  // dc(:) = H*(x(:).*dc(:))./Hs./max(1e-3,x(:));
  // max(1e-3,x(:))
  den_sense_val /= std::max(1e-3, _design_density.getElementalValue(_current_elem));

  _density_sensitivity.setNodalValue(den_sense_val);
}
