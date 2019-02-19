#include "CFLTimeStepSize.h"

#include <algorithm>
#include <limits>

#include "libmesh/quadrature.h"

registerMooseObject("THMApp", CFLTimeStepSize);

template <>
InputParameters
validParams<CFLTimeStepSize>()
{
  InputParameters params = validParams<ElementPostprocessor>();

  params.addParam<Real>("CFL", 0.5, "The CFL number to use in computing time step size");
  params.addRequiredParam<std::vector<MaterialPropertyName>>("vel_names",
                                                             "Velocity material property name(s)");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "c_names", "Sound speed material property name(s)");

  // Because this post-processor is meant to be used with PostprocessorDT, it
  // should be executed on initial (not included by default) and timestep end.
  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
  params.suppressParameter<ExecFlagEnum>("execute_on");

  params.addClassDescription("Computes a time step size based on user-specified CFL number");

  return params;
}

CFLTimeStepSize::CFLTimeStepSize(const InputParameters & parameters)
  : ElementPostprocessor(parameters),
    _CFL(getParam<Real>("CFL")),
    _vel_names(getParam<std::vector<MaterialPropertyName>>("vel_names")),
    _c_names(getParam<std::vector<MaterialPropertyName>>("c_names")),
    _n_phases(_vel_names.size()),
    _dt(std::numeric_limits<Real>::max())
{
  if (_vel_names.size() != _c_names.size())
    mooseError("The number of elements in the parameters 'vel_names' and 'c_names' must be equal.");

  for (unsigned int k = 0; k < _n_phases; ++k)
  {
    _vel.push_back(&getMaterialPropertyByName<Real>(_vel_names[k]));
    _c.push_back(&getMaterialPropertyByName<Real>(_c_names[k]));
  }
}

void
CFLTimeStepSize::initialize()
{
  // start with the max
  _dt = std::numeric_limits<Real>::max();
}

Real
CFLTimeStepSize::getValue()
{
  gatherMin(_dt);
  return _dt;
}

void
CFLTimeStepSize::threadJoin(const UserObject & y)
{
  const CFLTimeStepSize & pps = static_cast<const CFLTimeStepSize &>(y);

  _dt = std::min(_dt, pps._dt);
}

void
CFLTimeStepSize::execute()
{
  // get minimum element diameter for element
  const Real h_min_element = _current_elem->hmin();

  // loop over quadrature points
  for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
  {
    // determine minimum time step size over all phases
    for (unsigned int k = 0; k < _n_phases; ++k)
    {
      const Real dt_phase = _CFL * h_min_element / (std::fabs((*_vel[k])[qp]) + (*_c[k])[qp]);
      _dt = std::min(_dt, dt_phase);
    }
  }
}
