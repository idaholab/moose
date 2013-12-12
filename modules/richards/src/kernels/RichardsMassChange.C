#include "RichardsMassChange.h"
#include "Material.h"

#include <iostream>


template<>
InputParameters validParams<RichardsMassChange>()
{
  InputParameters params = validParams<Kernel>();
  params.addParam<bool>("lumping", false, "True for mass matrix lumping, false otherwise.  NOTE: THIS CURRENTLY DOES NOTHING!");
  params.addParam<bool>("use_supg", false, "True for using SUPG in this kernel, false otherwise.  This has no effect if the material does not use SUPG.");
  return params;
}

RichardsMassChange::RichardsMassChange(const std::string & name,
                                             InputParameters parameters) :
    Kernel(name,parameters),

    _this_var_num(_var.index()),

    _lumping(getParam<bool>("lumping")),
    _use_supg(getParam<bool>("use_supg")),
    // This kernel expects input parameters named "bulk_mod", etc
    _porosity(getMaterialProperty<Real>("porosity")),

    _p_var_nums(getMaterialProperty<std::vector<unsigned int> >("p_var_nums")),

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
  for (int pvar=0 ; pvar<_p_var_nums.size() ; ++pvar )
    {
      if (_p_var_nums[_qp][pvar] == _this_var_num)
	{
	  Real mass = _porosity[_qp]*_density[_qp][pvar]*_sat[_qp][pvar];
	  Real mass_old = _porosity[_qp]*_density_old[_qp][pvar]*_sat_old[_qp][pvar];
	  
	  Real test_fcn = _test[_i][_qp] ;
	  if (_use_supg) {
	    test_fcn += _tauvel_SUPG[_qp][pvar]*_grad_test[_i][_qp];
	  }
	  return test_fcn*(mass - mass_old)/_dt;
	}
    }
  return 0.0;
}

Real
RichardsMassChange::computeQpJacobian()
{
  for (int pvar=0 ; pvar<_p_var_nums.size() ; ++pvar )
    {
      if (_p_var_nums[_qp][pvar] == _this_var_num)
	{
	  Real mass = _porosity[_qp]*_density[_qp][pvar]*_sat[_qp][pvar];
	  Real mass_old = _porosity[_qp]*_density_old[_qp][pvar]*_sat_old[_qp][pvar];
	  Real mass_prime = _phi[_j][_qp]*_porosity[_qp]*(_ddensity[_qp][pvar]*_sat[_qp][pvar] + _density[_qp][pvar]*_dsat[_qp][pvar][pvar]);

	  //std::cout << _ddensity[_qp][pvar] << " " << _sat[_qp][pvar]  << " " <<  _density[_qp][pvar] << " " << _dsat[_qp][pvar][pvar] << "\n";
	  //std::cout << "phi=" << _phi[_j][_qp] << " " << _test[_i][_qp] << "\n";

	  Real test_fcn = _test[_i][_qp] ;
	  Real test_fcn_prime = 0;
	  
	  if (_use_supg) {
	    test_fcn += _tauvel_SUPG[_qp][pvar]*_grad_test[_i][_qp];
	    test_fcn_prime += _grad_phi[_j][_qp]*(_dtauvel_SUPG_dgradp[_qp][pvar]*_grad_test[_i][_qp]) + _phi[_j][_qp]*_dtauvel_SUPG_dp[_qp][pvar]*_grad_test[_i][_qp];
	  }
	  //std::cout << (test_fcn*mass_prime + test_fcn_prime*(mass- mass_old))/_dt << " " << _dt << "\n";
	  return (test_fcn*mass_prime + test_fcn_prime*(mass- mass_old))/_dt;
	}
    }
  return 0.0;
}

Real
RichardsMassChange::computeQpOffDiagJacobian(unsigned int jvar)
{
  for (int pvar=0 ; pvar<_p_var_nums.size() ; ++pvar )
    {
      if (_p_var_nums[_qp][pvar] == _this_var_num)
	{
	  for (int dvar=0 ; dvar<_p_var_nums.size() ; ++dvar )
	    {
	      if (_p_var_nums[_qp][dvar] == jvar)
		{
		  Real mass_prime = _phi[_j][_qp]*_porosity[_qp]*_density[_qp][pvar]*_dsat[_qp][pvar][dvar];
		  Real test_fcn = _test[_i][_qp] ;
		  if (_use_supg) {
		    test_fcn += _tauvel_SUPG[_qp][pvar]*_grad_test[_i][_qp];
		  }
		  return test_fcn*mass_prime/_dt;
		}
	    }
	}
    }
  return 0.0;
}
