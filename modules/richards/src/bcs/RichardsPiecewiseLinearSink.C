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

    _pp(getMaterialProperty<std::vector<Real> >("porepressure")),
    _dpp_dv(getMaterialProperty<std::vector<std::vector<Real> > >("dporepressure_dv")),

    _viscosity(getMaterialProperty<std::vector<Real> >("viscosity")),
    _permeability(getMaterialProperty<RealTensorValue>("permeability")),

    _dseff_dv(getMaterialProperty<std::vector<std::vector<Real> > >("ds_eff_dv")),

    _rel_perm(getMaterialProperty<std::vector<Real> >("rel_perm")),
    _drel_perm_dv(getMaterialProperty<std::vector<std::vector<Real> > >("drel_perm_dv")),

    _density(getMaterialProperty<std::vector<Real> >("density")),
    _ddensity_dv(getMaterialProperty<std::vector<std::vector<Real> > >("ddensity_dv"))
{}



Real
RichardsPiecewiseLinearSink::computeQpResidual()
{
  Real flux = _test[_i][_qp]*_sink_func.sample(_pp[_qp][_pvar]);
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
  return jac(_pvar);
}

Real
RichardsPiecewiseLinearSink::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (_richards_name_UO.not_richards_var(jvar))
    return 0.0;
  unsigned int dvar = _richards_name_UO.richards_var_num(jvar);
  return jac(dvar);
}

Real
RichardsPiecewiseLinearSink::jac(unsigned int wrt_num)
{
  Real flux = _sink_func.sample(_pp[_qp][_pvar]);
  Real deriv = _sink_func.sampleDerivative(_pp[_qp][_pvar])*_dpp_dv[_qp][_pvar][wrt_num];
  if (_use_mobility)
    {
      Real k = (_permeability[_qp]*_normals[_qp])*_normals[_qp];
      Real mob = _density[_qp][_pvar]*k/_viscosity[_qp][_pvar];
      Real mobp = _ddensity_dv[_qp][_pvar][wrt_num]*k/_viscosity[_qp][_pvar];
      deriv = mob*deriv + mobp*flux;
      flux *= mob;
    }
  if (_use_relperm)
    {
      deriv = _rel_perm[_qp][_pvar]*deriv + _drel_perm_dv[_qp][_pvar][wrt_num]*flux;
    }

  deriv *= _m_func.value(_t, _q_point[_qp]);

  return _test[_i][_qp]*deriv*_phi[_j][_qp];
}
