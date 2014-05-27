/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#include "RichardsPiecewiseLinearSink.h"

#include <iostream>


template<>
InputParameters validParams<RichardsPiecewiseLinearSink>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addRequiredParam<bool>("use_mobility", "If true, then fluxes are multiplied by (density*permeability_nn/viscosity), where the '_nn' indicates the component normal to the boundary.  In this case bare_flux is measured in Pa.s^-1.  This can be used in conjunction with use_relperm.");
  params.addRequiredParam<bool>("use_relperm", "If true, then fluxes are multiplied by relative permeability.  This can be used in conjunction with use_mobility");
  params.addRequiredParam<std::vector<Real> >("pressures", "Tuple of pressure values.  Must be monotonically increasing.");
  params.addRequiredParam<std::vector<Real> >("bare_fluxes", "Tuple of flux values (measured in kg.m^-2.s^-1 for use_mobility=false, and in Pa.s^-1 if use_mobility=true).  A piecewise-linear fit is performed to the (pressure,bare_fluxes) pairs to obtain the flux at any arbitrary pressure.  If a quad-point pressure is less than the first pressure value, the first bare_flux value is used.  If quad-point pressure exceeds the final pressure value, the final bare_flux value is used.  This flux is OUT of the medium: hence positive values of flux means this will be a SINK, while negative values indicate this flux will be a SOURCE.");
  params.addParam<FunctionName>("multiplying_fcn", 1.0, "If this function is provided, the flux will be multiplied by this function.  This is useful for spatially or temporally varying sinks");
  params.addRequiredParam<UserObjectName>("richardsVarNames_UO", "The UserObject that holds the list of Richards variable names.");
  return params;
}

RichardsPiecewiseLinearSink::RichardsPiecewiseLinearSink(const std::string & name,
                                             InputParameters parameters) :
    IntegratedBC(name,parameters),
    _use_mobility(getParam<bool>("use_mobility")),
    _use_relperm(getParam<bool>("use_relperm")),
    _sink_func(getParam<std::vector<Real> >("pressures"), getParam<std::vector<Real> >("bare_fluxes")),

    _m_func(getFunction("multiplying_fcn")),

    _richards_name_UO(getUserObject<RichardsVarNames>("richardsVarNames_UO")),
    _pvar(_richards_name_UO.richards_var_num(_var.number())),

    _viscosity(getMaterialProperty<std::vector<Real> >("viscosity")),
    _permeability(getMaterialProperty<RealTensorValue>("permeability")),

    _dseff(getMaterialProperty<std::vector<std::vector<Real> > >("ds_eff")),

    _rel_perm(getMaterialProperty<std::vector<Real> >("rel_perm")),
    _drel_perm(getMaterialProperty<std::vector<Real> >("drel_perm")),

    _density(getMaterialProperty<std::vector<Real> >("density")),
    _ddensity(getMaterialProperty<std::vector<Real> >("ddensity"))
{}



Real
RichardsPiecewiseLinearSink::computeQpResidual()
{
  Real flux = _test[_i][_qp]*_sink_func.sample(_u[_qp]);
  if (_use_mobility)
    {
      Real k = (_permeability[_qp]*_normals[_qp])*_normals[_qp];
      flux *= _density[_qp][_pvar]*k/_viscosity[_qp][_pvar];
    }
  if (_use_relperm)
    flux *= _rel_perm[_qp][_pvar];

  flux *= _m_func.value(_t, _q_point[_qp]);

  return flux;
}

Real
RichardsPiecewiseLinearSink::computeQpJacobian()
{
  Real flux = _sink_func.sample(_u[_qp]);
  Real deriv = _sink_func.sampleDerivative(_u[_qp]);
  if (_use_mobility)
    {
      Real k = (_permeability[_qp]*_normals[_qp])*_normals[_qp];
      Real mob = _density[_qp][_pvar]*k/_viscosity[_qp][_pvar];
      Real mobp = _ddensity[_qp][_pvar]*k/_viscosity[_qp][_pvar];
      deriv = mob*deriv + mobp*flux;
      flux = mob*flux;
    }
  if (_use_relperm)
    {
      deriv = _rel_perm[_qp][_pvar]*deriv + _drel_perm[_qp][_pvar]*_dseff[_qp][_pvar][_pvar]*flux;
    }

  deriv *= _m_func.value(_t, _q_point[_qp]);

  return _test[_i][_qp]*deriv*_phi[_j][_qp];
}

Real
RichardsPiecewiseLinearSink::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (_richards_name_UO.not_richards_var(jvar) || !(_use_relperm))
    return 0.0;
  unsigned int dvar = _richards_name_UO.richards_var_num(jvar);

  // only relperm has off-diag contributions

  Real flux = _sink_func.sample(_u[_qp]);
  if (_use_mobility)
    {
      Real k = (_permeability[_qp]*_normals[_qp])*_normals[_qp];
      Real mob = _density[_qp][_pvar]*k/_viscosity[_qp][_pvar];
      flux = mob*flux;
    }
  Real deriv = _drel_perm[_qp][_pvar]*_dseff[_qp][_pvar][dvar]*flux;

  deriv *= _m_func.value(_t, _q_point[_qp]);

  return _test[_i][_qp]*deriv*_phi[_j][_qp];
}
