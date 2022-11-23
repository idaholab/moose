//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "CFLTimeStepSize.h"

// MOOSE includes
#include "libmesh/quadrature.h"
#include "metaphysicl/raw_type.h"

#include <algorithm>
#include <limits>

registerMooseObject("NavierStokesApp", CFLTimeStepSize);
registerMooseObject("NavierStokesApp", ADCFLTimeStepSize);

template <bool is_ad>
InputParameters
CFLTimeStepSizeTempl<is_ad>::validParams()
{
  InputParameters params = ElementPostprocessor::validParams();

  params.addParam<Real>("CFL", 0.5, "The CFL number to use in computing time step size");
  params.addRequiredParam<std::vector<MaterialPropertyName>>("vel_names",
                                                             "Velocity material property name(s)");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "c_names", "Sound speed material property name(s)");

  // Because this post-processor is meant to be used with PostprocessorDT, it
  // should be executed on initial (not included by default) and timestep end.
  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
  params.suppressParameter<ExecFlagEnum>("execute_on");

  params.addClassDescription("Computes a time step size based on a user-specified CFL number");

  return params;
}

template <bool is_ad>
CFLTimeStepSizeTempl<is_ad>::CFLTimeStepSizeTempl(const InputParameters & parameters)
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
    _vel.push_back(&getGenericMaterialPropertyByName<Real, is_ad>(_vel_names[k]));
    _c.push_back(&getGenericMaterialPropertyByName<Real, is_ad>(_c_names[k]));
  }
}

template <bool is_ad>
void
CFLTimeStepSizeTempl<is_ad>::initialize()
{
  // start with the max
  _dt = std::numeric_limits<Real>::max();
}

template <bool is_ad>
Real
CFLTimeStepSizeTempl<is_ad>::getValue()
{
  return _dt;
}

template <bool is_ad>
void
CFLTimeStepSizeTempl<is_ad>::finalize()
{
  gatherMin(_dt);
}

template <bool is_ad>
void
CFLTimeStepSizeTempl<is_ad>::threadJoin(const UserObject & y)
{
  const CFLTimeStepSizeTempl<is_ad> & pps = static_cast<const CFLTimeStepSizeTempl<is_ad> &>(y);

  _dt = std::min(_dt, pps._dt);
}

template <bool is_ad>
void
CFLTimeStepSizeTempl<is_ad>::execute()
{
  // get minimum element diameter for element
  const Real h_min_element = _current_elem->hmin();

  // loop over quadrature points
  for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
  {
    // determine minimum time step size over all phases
    for (unsigned int k = 0; k < _n_phases; ++k)
    {
      const Real dt_phase = _CFL * h_min_element /
                            (std::fabs(MetaPhysicL::raw_value((*_vel[k])[qp])) +
                             MetaPhysicL::raw_value((*_c[k])[qp]));
      _dt = std::min(_dt, dt_phase);
    }
  }
}
