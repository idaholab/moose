#include <cmath> // std::sinh and std::cosh
#include "RichardsMaterial.h"



template<>
InputParameters validParams<RichardsMaterial>()
{
  InputParameters params = validParams<Material>();

  params.addRequiredParam<Real>("mat_porosity", "The porosity of the material.  Should be between 0 and 1.  Eg, 0.1");
  params.addRequiredParam<RealTensorValue>("mat_permeability", "The permeability tensor (m^2).");
  params.addRequiredParam<std::vector<UserObjectName> >("relperm_UO", "List of names of user objects that define relative permeability");
  params.addRequiredParam<std::vector<UserObjectName> >("seff_UO", "List of name of user objects that define effective saturation as a function of pressure list");
  params.addRequiredParam<std::vector<UserObjectName> >("sat_UO", "List of names of user objects that define saturation as a function of effective saturation");
  params.addRequiredParam<std::vector<UserObjectName> >("density_UO", "List of names of user objects that define the fluid density");
  params.addRequiredParam<std::vector<UserObjectName> >("SUPG_UO", "List of names of user objects that define the SUPG");
  params.addRequiredParam<std::vector<Real> >("viscosity", "List of viscosity of fluids (Pa.s).  Typical value for water is=1E-3");
  params.addRequiredParam<RealVectorValue>("gravity", "Gravitational acceleration (m/s^2) as a vector pointing downwards.  Eg (0,0,-10)");
  params.addRequiredCoupledVar("pressure_vars", "List of variables that represent the pressure");
  params.addParam<bool>("linear_shape_fcns", true, "If you are using second-order Lagrange shape functions you need to set this to false.");
  
  return params;
}

RichardsMaterial::RichardsMaterial(const std::string & name,
                                 InputParameters parameters) :
    Material(name, parameters),
    _material_por(getParam<Real>("mat_porosity")),
    _material_perm(getParam<RealTensorValue>("mat_permeability")),
    _material_viscosity(getParam<std::vector<Real> >("viscosity")),

    _num_p(coupledComponents("pressure_vars")),

    // FOLLOWING IS FOR SUPG
    _material_gravity(getParam<RealVectorValue>("gravity")),
    _trace_perm(_material_perm.tr()),

    // Grab reference to linear Lagrange finite element object pointer,
    // currently this is always a linear Lagrange element, so this might need to
    // be generalized if we start working with higher-order elements...
    _fe(_subproblem.assembly(_tid).getFE( getParam<bool>("linear_shape_fcns") ? FEType(FIRST, LAGRANGE) : FEType(SECOND, LAGRANGE))),
    
    // Grab references to FE object's mapping data from the _subproblem's FE object
    _dxidx(_fe->get_dxidx()),
    _dxidy(_fe->get_dxidy()),
    _dxidz(_fe->get_dxidz()),
    _detadx(_fe->get_detadx()),
    _detady(_fe->get_detady()),
    _detadz(_fe->get_detadz()),
    _dzetadx(_fe->get_dzetadx()), // Empty in 2D
    _dzetady(_fe->get_dzetady()), // Empty in 2D
    _dzetadz(_fe->get_dzetadz()), // Empty in 2D

    // Declare that this material is going to provide a Real
    // valued property named "porosity", etc, that Kernels can use.
    _porosity(declareProperty<Real>("porosity")),
    _permeability(declareProperty<RealTensorValue>("permeability")),

    _viscosity(declareProperty<std::vector<Real> >("viscosity")),
    _gravity(declareProperty<RealVectorValue>("gravity")),

    _p_var_nums(declareProperty<std::vector<unsigned int> >("p_var_nums")),
    _mat_var_num(declareProperty<std::vector<int> >("mat_var_num")),

    _density_old(declareProperty<std::vector<Real> >("density_old")),

    _density(declareProperty<std::vector<Real> >("density")),
    _ddensity(declareProperty<std::vector<Real> >("ddensity")),
    _d2density(declareProperty<std::vector<Real> >("d2density")),

    _seff_old(declareProperty<std::vector<Real> >("s_eff_old")),

    _seff(declareProperty<std::vector<Real> >("s_eff")),
    _dseff(declareProperty<std::vector<std::vector<Real> > >("ds_eff")),
    _d2seff(declareProperty<std::vector<std::vector<std::vector<Real> > > >("d2s_eff")),

    _sat_old(declareProperty<std::vector<Real> >("sat_old")),

    _sat(declareProperty<std::vector<Real> >("sat")),
    _dsat(declareProperty<std::vector<std::vector<Real> > >("dsat")),
    _d2sat(declareProperty<std::vector<std::vector<std::vector<Real> > > >("d2sat")),

    _rel_perm(declareProperty<std::vector<Real> >("rel_perm")),
    _drel_perm(declareProperty<std::vector<Real> >("drel_perm")),
    _d2rel_perm(declareProperty<std::vector<Real> >("d2rel_perm")),

    _tauvel_SUPG(declareProperty<std::vector<RealVectorValue> >("tauvel_SUPG")),
    _dtauvel_SUPG_dgradp(declareProperty<std::vector<RealTensorValue> >("dtauvel_SUPG_dgradp")),
    _dtauvel_SUPG_dp(declareProperty<std::vector<RealVectorValue> >("dtauvel_SUPG_dp"))

{
  if (_material_por <= 0 || _material_por >= 1)
    mooseError("Porosity set to " << _material_por << " but it must be between 0 and 1");

  unsigned int n = coupledComponents("pressure_vars");
  _pressure_vars.resize(n);
  _pressure_vals.resize(n);
  _pressure_old_vals.resize(n);
  _material_relperm_UO.resize(n);
  _material_seff_UO.resize(n);
  _material_sat_UO.resize(n);
  _material_density_UO.resize(n);
  _material_SUPG_UO.resize(n);
  _grad_p.resize(n);

  unsigned int max_moose_var_num_seen(0);
    
  for (unsigned int i=0 ; i<n; ++i)
    {
      _pressure_vars[i] = coupled("pressure_vars", i);
      max_moose_var_num_seen = (max_moose_var_num_seen > _pressure_vars[i] ? max_moose_var_num_seen : _pressure_vars[i]);
      _pressure_vals[i] = &coupledValue("pressure_vars", i);
      _pressure_old_vals[i] = (_is_transient ? &coupledValueOld("pressure_vars", i) : &_zero);
      _grad_p[i] = &coupledGradient("pressure_vars", i);
      // in the following.  first get the userobject names that were inputted, then get the i_th one of these, then get the actual userobject that this corresponds to, then finally & gives pointer to RichardsRelPerm object. 
      _material_relperm_UO[i] = &getUserObjectByName<RichardsRelPerm>(getParam<std::vector<UserObjectName> >("relperm_UO")[i]); 
      _material_seff_UO[i] = &getUserObjectByName<RichardsSeff>(getParam<std::vector<UserObjectName> >("seff_UO")[i]); 
      _material_sat_UO[i] = &getUserObjectByName<RichardsSat>(getParam<std::vector<UserObjectName> >("sat_UO")[i]); 
      _material_density_UO[i] = &getUserObjectByName<RichardsDensity>(getParam<std::vector<UserObjectName> >("density_UO")[i]); 
      _material_SUPG_UO[i] = &getUserObjectByName<RichardsSUPG>(getParam<std::vector<UserObjectName> >("SUPG_UO")[i]); 
    }
  
  _material_var_num.resize(max_moose_var_num_seen + 1);
  for (unsigned int i=0 ; i<max_moose_var_num_seen+1 ; ++i)
    _material_var_num[i] = -1;
  for (unsigned int i=0 ; i<n; ++i)
    {
      _material_var_num[_pressure_vars[i]] = i;
    }
      
}

/*
void
RichardsMaterial::computeQpProperties()
{
      _porosity[qp] = _material_por;
      _permeability[qp] = _material_perm;

      _viscosity[qp].resize(_num_p);
      _viscosity[qp] = _material_viscosity;

      _gravity[qp] = _material_gravity;

      for (unsigned int i=0 ; i<_num_p; ++i)
}
*/

void
RichardsMaterial::computeProperties()
{
  // this gets run for each element
  for (unsigned int qp=0; qp<_qrule->n_points(); qp++)
    {

      _porosity[qp] = _material_por;
      _permeability[qp] = _material_perm;
      _gravity[qp] = _material_gravity;

      _viscosity[qp].resize(_num_p);

      _p_var_nums[qp].resize(_num_p);
      _mat_var_num[qp].resize(_material_var_num.size());

      _density_old[qp].resize(_num_p);
      _density[qp].resize(_num_p);
      _ddensity[qp].resize(_num_p);
      _d2density[qp].resize(_num_p);

      _rel_perm[qp].resize(_num_p);
      _drel_perm[qp].resize(_num_p);
      _d2rel_perm[qp].resize(_num_p);

      _seff_old[qp].resize(_num_p);
      _seff[qp].resize(_num_p);
      _dseff[qp].resize(_num_p);
      _d2seff[qp].resize(_num_p);

      _sat_old[qp].resize(_num_p);
      _sat[qp].resize(_num_p);
      _dsat[qp].resize(_num_p);
      _d2sat[qp].resize(_num_p);


      for (unsigned int i=0 ; i<_num_p; ++i)
	{
	  _viscosity[qp][i] = _material_viscosity[i];

	  _p_var_nums[qp][i] = _pressure_vars[i];

	  _density_old[qp][i] = (*_material_density_UO[i]).density((*_pressure_old_vals[i])[qp]);
	  _density[qp][i] = (*_material_density_UO[i]).density((*_pressure_vals[i])[qp]);
	  _ddensity[qp][i] = (*_material_density_UO[i]).ddensity((*_pressure_vals[i])[qp]);
	  _d2density[qp][i] = (*_material_density_UO[i]).d2density((*_pressure_vals[i])[qp]);

	  _seff_old[qp][i] = (*_material_seff_UO[i]).seff(_pressure_old_vals, qp);
	  _seff[qp][i] = (*_material_seff_UO[i]).seff(_pressure_vals, qp);

	  _dseff[qp][i].resize(_num_p);
	  _dseff[qp][i] = (*_material_seff_UO[i]).dseff(_pressure_vals, qp);

	  _d2seff[qp][i].resize(_num_p);
	  for (unsigned int j=0 ; j<_num_p; ++j)
	    {
	      _d2seff[qp][i][j].resize(_num_p);
	    }
	  _d2seff[qp][i] = (*_material_seff_UO[i]).d2seff(_pressure_vals, qp);

	  _sat_old[qp][i] = (*_material_sat_UO[i]).sat(_seff_old[qp][i]);
	  _sat[qp][i] = (*_material_sat_UO[i]).sat(_seff[qp][i]);
	  _dsat[qp][i].resize(_num_p);
	  for (unsigned int j=0 ; j<_num_p; ++j)
	    {
	      _dsat[qp][i][j] = (*_material_sat_UO[i]).dsat(_seff[qp][i])*_dseff[qp][i][j]; // could optimise
	    }
	  _d2sat[qp][i].resize(_num_p);
	  for (unsigned int j=0 ; j<_num_p; ++j)
	    {
	      _d2sat[qp][i][j].resize(_num_p);
	      for (unsigned int k=0 ; k<_num_p; ++k)
		{
		  _d2sat[qp][i][j][k] = (*_material_sat_UO[i]).d2sat(_seff[qp][i])*_dseff[qp][i][j]*_dseff[qp][i][k] + (*_material_sat_UO[i]).dsat(_seff[qp][i])*_d2seff[qp][i][j][k];
		}
	    }

	  _rel_perm[qp][i] = (*_material_relperm_UO[i]).relperm(_seff[qp][i]);
	  _drel_perm[qp][i] = (*_material_relperm_UO[i]).drelperm(_seff[qp][i]);
	  _d2rel_perm[qp][i] =(* _material_relperm_UO[i]).d2relperm(_seff[qp][i]);

	}

      for (unsigned int i=0 ; i<_material_var_num.size(); ++i)
	{
	  _mat_var_num[qp][i] = _material_var_num[i];
	}


      // Now SUPG stuff
      _tauvel_SUPG[qp].resize(_num_p);
      _dtauvel_SUPG_dgradp[qp].resize(_num_p);
      _dtauvel_SUPG_dp[qp].resize(_num_p);
      
      // Bounds checking on element data and putting into vector form
      mooseAssert(qp < _dxidx.size(), "Insufficient data in dxidx array!");
      mooseAssert(qp < _dxidy.size(), "Insufficient data in dxidy array!");
      mooseAssert(qp < _dxidz.size(), "Insufficient data in dxidz array!");
      if (_mesh.dimension() >= 2)
	{
	  mooseAssert(qp < _detadx.size(), "Insufficient data in detadx array!");
	  mooseAssert(qp < _detady.size(), "Insufficient data in detady array!");
	  mooseAssert(qp < _detadz.size(), "Insufficient data in detadz array!");
	}
      if (_mesh.dimension() >= 3)
	{
	  mooseAssert(qp < _dzetadx.size(), "Insufficient data in dzetadx array!");
	  mooseAssert(qp < _dzetady.size(), "Insufficient data in dzetady array!");
	  mooseAssert(qp < _dzetadz.size(), "Insufficient data in dzetadz array!");
	}

      // CHECK : Does this work spherical, cylindrical, etc?
      RealVectorValue xi_prime(_dxidx[qp], _dxidy[qp], _dxidz[qp]);
      RealVectorValue eta_prime, zeta_prime;
      if (_mesh.dimension() >= 2)
	{
	  eta_prime(0) = _detadx[qp];
	  eta_prime(1) = _detady[qp];
	}
      if (_mesh.dimension() == 3)
	{
	  eta_prime(2) = _detadz[qp];
	  zeta_prime(0) = _dzetadx[qp];
	  zeta_prime(1) = _dzetady[qp];
	  zeta_prime(2) = _dzetadz[qp];
	}

      for (unsigned int i=0 ; i<_num_p; ++i)
	{
	  RealVectorValue vel = (*_material_SUPG_UO[i]).velSUPG(_permeability[qp], (*_grad_p[i])[qp], _density[qp][i], _gravity[qp]);
	  RealTensorValue dvel_dgradp = (*_material_SUPG_UO[i]).dvelSUPG_dgradp(_permeability[qp]);
	  RealVectorValue dvel_dp = (*_material_SUPG_UO[i]).dvelSUPG_dp(_permeability[qp], _ddensity[qp][i], _gravity[qp]);
	  RealVectorValue bb = (*_material_SUPG_UO[i]).bb(vel, _mesh.dimension(), xi_prime, eta_prime, zeta_prime);
	  RealVectorValue dbb2_dgradp = (*_material_SUPG_UO[i]).dbb2_dgradp(vel, dvel_dgradp, xi_prime, eta_prime, zeta_prime);
	  Real dbb2_dp = (*_material_SUPG_UO[i]).dbb2_dp(vel, dvel_dp, xi_prime, eta_prime, zeta_prime);
	  Real tau = (*_material_SUPG_UO[i]).tauSUPG(vel, _trace_perm, bb);
	  RealVectorValue dtau_dgradp = (*_material_SUPG_UO[i]).dtauSUPG_dgradp(vel, dvel_dgradp, _trace_perm, bb, dbb2_dgradp);
	  Real dtau_dp = (*_material_SUPG_UO[i]).dtauSUPG_dp(vel, dvel_dp, _trace_perm, bb, dbb2_dp);
	  _tauvel_SUPG[qp][i] = tau*vel;
	  _dtauvel_SUPG_dgradp[qp][i] = tau*dvel_dgradp;
	  for (unsigned int j=0 ; j<vel.size(); ++j)
	    for (unsigned int k=0 ; k<vel.size(); ++k)
	      _dtauvel_SUPG_dgradp[qp][i](j,k) += dtau_dgradp(j)*vel(k); // this is outerproduct - maybe libmesh can do it better?
	  _dtauvel_SUPG_dp[qp][i] = dtau_dp*vel + tau*dvel_dp;
	}
    }
}


