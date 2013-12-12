#include "RichardsFlux.h"
#include "Material.h"

#include <iostream>


template<>
InputParameters validParams<RichardsFlux>()
{
  InputParameters params = validParams<Kernel>();
  params.addParam<bool>("linear_shape_fcns", true, "If you are using second-order Lagrange shape functions you need to set this to false.");
  return params;
}

RichardsFlux::RichardsFlux(const std::string & name,
                                             InputParameters parameters) :
    Kernel(name,parameters),

    _this_var_num(_var.index()),

    // This kernel gets lots of things from the material
    _viscosity(getMaterialProperty<std::vector<Real> >("viscosity")),
    _gravity(getMaterialProperty<RealVectorValue>("gravity")),
    _permeability(getMaterialProperty<RealTensorValue>("permeability")),

    _p_var_nums(getMaterialProperty<std::vector<unsigned int> >("p_var_nums")),

    _seff(getMaterialProperty<std::vector<Real> >("s_eff")), // not actually used
    _dseff(getMaterialProperty<std::vector<std::vector<Real> > >("ds_eff")),
    _d2seff(getMaterialProperty<std::vector<std::vector<std::vector<Real> > > >("d2s_eff")),

    _rel_perm(getMaterialProperty<std::vector<Real> >("rel_perm")),
    _drel_perm(getMaterialProperty<std::vector<Real> >("drel_perm")),
    _d2rel_perm(getMaterialProperty<std::vector<Real> >("d2rel_perm")),

    _density(getMaterialProperty<std::vector<Real> >("density")), 
    _ddensity(getMaterialProperty<std::vector<Real> >("ddensity")),
    _d2density(getMaterialProperty<std::vector<Real> >("d2density")),

    _second_u(getParam<bool>("linear_shape_fcns") ? _second_zero : (_is_implicit ? _var.secondSln() : _var.secondSlnOld())),
    //_second_phi(getParam<bool>("linear_shape_fcns") ? _second_zero : secondPhi()), // does not compile
    _second_phi(secondPhi()), // does compile

    _tauvel_SUPG(getMaterialProperty<std::vector<RealVectorValue> >("tauvel_SUPG")),
    _dtauvel_SUPG_dgradp(getMaterialProperty<std::vector<RealTensorValue> >("dtauvel_SUPG_dgradp")),
    _dtauvel_SUPG_dp(getMaterialProperty<std::vector<RealVectorValue> >("dtauvel_SUPG_dp"))

{
}


Real
RichardsFlux::computeQpResidual()
{
  for (int pvar=0 ; pvar<_p_var_nums.size() ; ++pvar )
    {
      if (_p_var_nums[_qp][pvar] == _this_var_num)
	{
	  Real mob = mobility(_density[_qp][pvar], _rel_perm[_qp][pvar]);
	  RealVectorValue pot = _permeability[_qp]*(_grad_u[_qp] - _density[_qp][pvar]*_gravity[_qp]);
	  Real flux_part = _grad_test[_i][_qp]*mob*pot;

	  Real supg_test = _tauvel_SUPG[_qp][pvar]*_grad_test[_i][_qp];
	  Real supg_kernel = 0.0;
	  
	  if (supg_test != 0)
	    {
	      Real dmob_dp = dmobility_dp(_density[_qp][pvar], _ddensity[_qp][pvar], _rel_perm[_qp][pvar], _drel_perm[_qp][pvar]*_dseff[_qp][pvar][pvar]);
	      RealVectorValue grad_mob = dmob_dp*_grad_u[_qp];
	      // NOTE: since Libmesh does not correctly calculate grad(_grad_u) correctly, so following might not be correct
	      Real div_pot = (_permeability[_qp]*_second_u[_qp]).tr() - (_permeability[_qp]*_grad_u[_qp])*_ddensity[_qp][pvar]*_gravity[_qp];
	      supg_kernel = -grad_mob*pot - mob*div_pot;
	    }

	  return (flux_part + supg_test*supg_kernel)/_viscosity[_qp][pvar]; // TEST
	}
    }
  return 0.0;
}


Real
RichardsFlux::computeQpJacobian()
{
  for (int pvar=0 ; pvar<_p_var_nums.size() ; ++pvar )
    {
      if (_p_var_nums[_qp][pvar] == _this_var_num)
	{
	  Real mob = mobility(_density[_qp][pvar], _rel_perm[_qp][pvar]);
	  Real dmob_dp = dmobility_dp(_density[_qp][pvar], _ddensity[_qp][pvar], _rel_perm[_qp][pvar], _drel_perm[_qp][pvar]*_dseff[_qp][pvar][pvar]);
	  RealVectorValue pot = _permeability[_qp]*(_grad_u[_qp] - _density[_qp][pvar]*_gravity[_qp]);
	  RealVectorValue dpot_dp = _permeability[_qp]*(_grad_phi[_j][_qp] - _phi[_j][_qp]*_ddensity[_qp][pvar]*_gravity[_qp]); // note: includes _phi

	  Real dflux_dp = _grad_test[_i][_qp]*(dmob_dp*_phi[_j][_qp]*pot + mob*dpot_dp);

	  Real supg_test = _tauvel_SUPG[_qp][pvar]*_grad_test[_i][_qp];
	  Real supg_test_prime = _grad_phi[_j][_qp]*(_dtauvel_SUPG_dgradp[_qp][pvar]*_grad_test[_i][_qp]) + _phi[_j][_qp]*_dtauvel_SUPG_dp[_qp][pvar]*_grad_test[_i][_qp];
	  Real supg_kernel = 0.0;
	  Real supg_kernel_prime = 0.0;

	  if (supg_test != 0)
	    {
	      RealVectorValue grad_mob = dmob_dp*_grad_u[_qp];
	      // NOTE: since Libmesh does not correctly calculate grad(_grad_u) correctly, so following might not be correct
	      Real div_pot = ((_permeability[_qp]*_second_u[_qp]).tr() - (_permeability[_qp]*_grad_u[_qp])*_ddensity[_qp][pvar]*_gravity[_qp]);
	      supg_kernel = -grad_mob*pot - mob*div_pot;

	      Real d2mob_dp2 = d2mobility_dp2(_density[_qp][pvar], _ddensity[_qp][pvar], _d2density[_qp][pvar], _rel_perm[_qp][pvar], _drel_perm[_qp][pvar]*_dseff[_qp][pvar][pvar], _d2rel_perm[_qp][pvar]*_dseff[_qp][pvar][pvar]*_dseff[_qp][pvar][pvar] + _drel_perm[_qp][pvar]*_d2seff[_qp][pvar][pvar][pvar]);
	      RealVectorValue dgrad_mob_dp = d2mob_dp2*_phi[_j][_qp]*_grad_u[_qp] + dmob_dp*_grad_phi[_j][_qp];
	      Real ddiv_pot_dp = -(_permeability[_qp]*_grad_phi[_j][_qp])*_ddensity[_qp][pvar]*_gravity[_qp]  - (_permeability[_qp]*_grad_u[_qp])*_d2density[_qp][pvar]*_phi[_j][_qp]*_gravity[_qp];
	      //Real ddiv_pot_dp += (_permeability[_qp]*_second_phi[_j][_qp]).tr();

	      supg_kernel_prime = -dgrad_mob_dp*pot - grad_mob*dpot_dp - dmob_dp*_phi[_j][_qp]*div_pot - mob*ddiv_pot_dp;
	    }

	  return (dflux_dp + supg_test_prime*supg_kernel + supg_test*supg_kernel_prime)/_viscosity[_qp][pvar]; // TEST
	}
    }
  return 0.0;
}


Real
RichardsFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  for (int pvar=0 ; pvar<_p_var_nums.size() ; ++pvar )
    {
      if (_p_var_nums[_qp][pvar] == _this_var_num)
	{
	  for (int dvar=0 ; dvar<_p_var_nums.size() ; ++dvar )
	    {
	      if (_p_var_nums[_qp][dvar] == jvar)
		{
		  Real flux_prime = _grad_test[_i][_qp]*(_density[_qp][pvar]*_drel_perm[_qp][pvar]*_dseff[_qp][pvar][dvar]*_phi[_j][_qp]/_viscosity[_qp][pvar]*(_permeability[_qp]*(_grad_u[_qp] - _density[_qp][pvar]*_gravity[_qp])));
		  
		  Real supg_test = _tauvel_SUPG[_qp][pvar]*_grad_test[_i][_qp];
		  Real supg_test_prime = 0.0;
		  Real supg_kernel = 0.0;
		  Real supg_kernel_prime = 0.0;
		  if (supg_test != 0)
		    {
		      //supg_kernel = -(_ddensity[_qp][pvar]*_rel_perm[_qp][pvar] + _density[_qp][pvar]*_drel_perm[_qp][pvar]*_dseff[_qp][pvar][pvar])/_viscosity[_qp][pvar]*_grad_u[_qp]*(_permeability[_qp]*(_grad_u[_qp] - _density[_qp][pvar]*_gravity[_qp]));
		      //supg_kernel -= (_density[_qp][pvar]*_rel_perm[_qp][pvar]/_viscosity[_qp][pvar])*((_permeability[_qp]*_second_u[_qp]).tr() - (_permeability[_qp]*_grad_u[_qp])*_ddensity[_qp][pvar]*_gravity[_qp]);
		      supg_kernel_prime = -(_ddensity[_qp][pvar]*_drel_perm[_qp][pvar]*_dseff[_qp][pvar][dvar] + _density[_qp][pvar]*_d2rel_perm[_qp][pvar]*_dseff[_qp][pvar][dvar]*_dseff[_qp][pvar][pvar] + _density[_qp][pvar]*_drel_perm[_qp][pvar]*_d2seff[_qp][pvar][pvar][dvar])*_phi[_j][_qp]/_viscosity[_qp][pvar]*_grad_u[_qp]*(_permeability[_qp]*(_grad_u[_qp] - _density[_qp][pvar]*_gravity[_qp]));
		      supg_kernel_prime -= _density[_qp][pvar]*_drel_perm[_qp][pvar]*_dseff[_qp][pvar][dvar]*_phi[_j][_qp]/_viscosity[_qp][pvar]*((_permeability[_qp]*_second_u[_qp]).tr() - (_permeability[_qp]*_grad_u[_qp])*_ddensity[_qp][pvar]*_gravity[_qp]);
		    }
		  return flux_prime + (supg_test_prime*supg_kernel + supg_test*supg_kernel_prime);
		}
	    }
	}
    }
  return 0.0;
}    


// mobility
Real
RichardsFlux::mobility(Real density, Real relperm)
{
  return density*relperm;
}

// d(mobility)/dp
Real
RichardsFlux::dmobility_dp(Real density, Real ddensity_dp, Real relperm, Real drelperm_dp)
{
  return ddensity_dp*relperm + density*drelperm_dp;
}

// d2(mobility)/dp2
Real
RichardsFlux::d2mobility_dp2(Real density, Real ddensity_dp, Real d2density_dp2, Real relperm, Real drelperm_dp, Real d2relperm_dp2)
{
  return d2density_dp2*relperm + 2*ddensity_dp*drelperm_dp + density*d2relperm_dp2;
}
