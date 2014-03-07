/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

//  This post processor returns the mass due to a flux from the boundary of a volume.
//
#include "RichardsPiecewiseLinearSinkFlux.h"

template<>
InputParameters validParams<RichardsPiecewiseLinearSinkFlux>()
{
  InputParameters params = validParams<SideIntegralVariablePostprocessor>();
  params.addRequiredParam<bool>("use_mobility", "If true, then fluxes are multiplied by (density*permeability_nn/viscosity), where the '_nn' indicates the component normal to the boundary.  In this case bare_flux is measured in Pa.s^-1.  This can be used in conjunction with use_relperm.");
  params.addRequiredParam<bool>("use_relperm", "If true, then fluxes are multiplied by relative permeability.  This can be used in conjunction with use_mobility");
  params.addRequiredParam<std::vector<Real> >("pressures", "Tuple of pressure values.  Must be monotonically increasing.");
  params.addRequiredParam<std::vector<Real> >("bare_fluxes", "Tuple of flux values (measured in kg.m^-2.s^-1 for use_mobility=false, and in Pa.s^-1 if use_mobility=true).  A piecewise-linear fit is performed to the (pressure,bare_fluxes) pairs to obtain the flux at any arbitrary pressure.  If a quad-point pressure is less than the first pressure value, the first bare_flux value is used.  If quad-point pressure exceeds the final pressure value, the final bare_flux value is used.  This flux is OUT of the medium: hence positive values of flux means this will be a SINK, while negative values indicate this flux will be a SOURCE.");
  params.addRequiredParam<UserObjectName>("porepressureNames_UO", "The UserObject that holds the list of porepressure names.");
  return params;
}

RichardsPiecewiseLinearSinkFlux::RichardsPiecewiseLinearSinkFlux(const std::string & name, InputParameters parameters) :
    SideIntegralVariablePostprocessor(name, parameters),
    _feproblem(dynamic_cast<FEProblem &>(_subproblem)),
    _use_mobility(getParam<bool>("use_mobility")),
    _use_relperm(getParam<bool>("use_relperm")),
    _sink_func(getParam<std::vector<Real> >("pressures"), getParam<std::vector<Real> >("bare_fluxes")),

    _pp_name_UO(getUserObject<RichardsPorepressureNames>("porepressureNames_UO")),
    _pvar(_pp_name_UO.pressure_var_num(_var.index())),

    _viscosity(getMaterialProperty<std::vector<Real> >("viscosity")),
    _permeability(getMaterialProperty<RealTensorValue>("permeability")),
    _rel_perm(getMaterialProperty<std::vector<Real> >("rel_perm")),
    _density(getMaterialProperty<std::vector<Real> >("density"))
{}

Real
RichardsPiecewiseLinearSinkFlux::computeQpIntegral()
{
  Real flux = _sink_func.sample(_u[_qp]);
  if (_use_mobility)
    {
      Real k = (_permeability[_qp]*_normals[_qp])*_normals[_qp];
      flux *= _density[_qp][_pvar]*k/_viscosity[_qp][_pvar];
    }
  if (_use_relperm)
    flux *= _rel_perm[_qp][_pvar];

  return flux*_feproblem.dt();
}
