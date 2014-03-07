/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#include "RichardsMassChange.h"
#include "Material.h"

#include <iostream>


template<>
InputParameters validParams<RichardsMassChange>()
{
  InputParameters params = validParams<TimeDerivative>();
  params.addParam<bool>("use_supg", false, "True for using SUPG in this kernel, false otherwise.  This has no effect if the material does not use SUPG.");
  params.addRequiredParam<UserObjectName>("porepressureNames_UO", "The UserObject that holds the list of porepressure names.");
  return params;
}

RichardsMassChange::RichardsMassChange(const std::string & name,
                                             InputParameters parameters) :
    TimeDerivative(name,parameters),
    _pp_name_UO(getUserObject<RichardsPorepressureNames>("porepressureNames_UO")),
    _pvar(_pp_name_UO.pressure_var_num(_var.index())),

    _use_supg(getParam<bool>("use_supg")),
    // This kernel expects input parameters named "bulk_mod", etc
    _porosity(getMaterialProperty<Real>("porosity")),
    _porosity_old(getMaterialProperty<Real>("porosity_old")),

    _sat_old(getMaterialProperty<std::vector<Real> >("sat_old")),

    _sat(getMaterialProperty<std::vector<Real> >("sat")),
    _dsat(getMaterialProperty<std::vector<std::vector<Real> > >("dsat")),
    _d2sat(getMaterialProperty<std::vector<std::vector<std::vector<Real> > > >("d2sat")),

    _density_old(getMaterialProperty<std::vector<Real> >("density_old")),

    _density(getMaterialProperty<std::vector<Real> >("density")),
    _ddensity(getMaterialProperty<std::vector<Real> >("ddensity")),
    _d2density(getMaterialProperty<std::vector<Real> >("d2density")),

    _tauvel_SUPG(getMaterialProperty<std::vector<RealVectorValue> >("tauvel_SUPG")),
    _dtauvel_SUPG_dgradp(getMaterialProperty<std::vector<RealTensorValue> >("dtauvel_SUPG_dgradp")),
    _dtauvel_SUPG_dp(getMaterialProperty<std::vector<RealVectorValue> >("dtauvel_SUPG_dp"))
{
}


Real
RichardsMassChange::computeQpResidual()
{
  Real mass = _porosity[_qp]*_density[_qp][_pvar]*_sat[_qp][_pvar];
  Real mass_old = _porosity_old[_qp]*_density_old[_qp][_pvar]*_sat_old[_qp][_pvar];

  Real test_fcn = _test[_i][_qp] ;
  if (_use_supg) {
    test_fcn += _tauvel_SUPG[_qp][_pvar]*_grad_test[_i][_qp];
  }
  return test_fcn*(mass - mass_old)/_dt;
}

Real
RichardsMassChange::computeQpJacobian()
{
  Real mass = _porosity[_qp]*_density[_qp][_pvar]*_sat[_qp][_pvar];
  Real mass_old = _porosity_old[_qp]*_density_old[_qp][_pvar]*_sat_old[_qp][_pvar];
  Real mass_prime = _phi[_j][_qp]*_porosity[_qp]*(_ddensity[_qp][_pvar]*_sat[_qp][_pvar] + _density[_qp][_pvar]*_dsat[_qp][_pvar][_pvar]);

  //Moose::out << _ddensity[_qp][_pvar] << " " << _sat[_qp][_pvar]  << " " <<  _density[_qp][_pvar] << " " << _dsat[_qp][_pvar][_pvar] << "\n";
  //Moose::out << "phi=" << _phi[_j][_qp] << " " << _test[_i][_qp] << "\n";

  Real test_fcn = _test[_i][_qp] ;
  Real test_fcn_prime = 0;

  if (_use_supg) {
    test_fcn += _tauvel_SUPG[_qp][_pvar]*_grad_test[_i][_qp];
    test_fcn_prime += _grad_phi[_j][_qp]*(_dtauvel_SUPG_dgradp[_qp][_pvar]*_grad_test[_i][_qp]) + _phi[_j][_qp]*_dtauvel_SUPG_dp[_qp][_pvar]*_grad_test[_i][_qp];
  }
  //Moose::out << (test_fcn*mass_prime + test_fcn_prime*(mass- mass_old))/_dt << " " << _dt << "\n";
  return (test_fcn*mass_prime + test_fcn_prime*(mass- mass_old))/_dt;
}

Real
RichardsMassChange::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (_pp_name_UO.not_pressure_var(jvar))
    return 0.0;
  unsigned int dvar = _pp_name_UO.pressure_var_num(jvar);
  Real mass_prime = _phi[_j][_qp]*_porosity[_qp]*_density[_qp][_pvar]*_dsat[_qp][_pvar][dvar];
  Real test_fcn = _test[_i][_qp] ;
  if (_use_supg) {
    test_fcn += _tauvel_SUPG[_qp][_pvar]*_grad_test[_i][_qp];
  }
  return test_fcn*mass_prime/_dt;
}
