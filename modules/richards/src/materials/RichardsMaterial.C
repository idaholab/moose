/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#include <cmath> // std::sinh and std::cosh
#include "RichardsMaterial.h"



template<>
InputParameters validParams<RichardsMaterial>()
{
  InputParameters params = validParams<Material>();

  params.addRequiredParam<Real>("mat_porosity", "The porosity of the material.  Should be between 0 and 1.  Eg, 0.1");
  params.addCoupledVar("por_change", "An auxillary variable describing porosity changes.  Porosity = mat_porosity + por_change.  If this is not provided, zero is used.");
  params.addRequiredParam<RealTensorValue>("mat_permeability", "The permeability tensor (m^2).");
  params.addCoupledVar("perm_change", "A list of auxillary variable describing permeability changes.  There must be 9 of these, corresponding to the xx, xy, xz, yx, yy, yz, zx, zy, zz components respectively.  Permeability = mat_permeability*10^(perm_change).");
  params.addRequiredParam<UserObjectName>("richardsVarNames_UO", "The UserObject that holds the list of Richards variable names.");
  params.addRequiredParam<std::vector<UserObjectName> >("relperm_UO", "List of names of user objects that define relative permeability");
  params.addRequiredParam<std::vector<UserObjectName> >("seff_UO", "List of name of user objects that define effective saturation as a function of pressure list");
  params.addRequiredParam<std::vector<UserObjectName> >("sat_UO", "List of names of user objects that define saturation as a function of effective saturation");
  params.addRequiredParam<std::vector<UserObjectName> >("density_UO", "List of names of user objects that define the fluid density");
  params.addRequiredParam<std::vector<UserObjectName> >("SUPG_UO", "List of names of user objects that define the SUPG");
  params.addRequiredParam<std::vector<Real> >("viscosity", "List of viscosity of fluids (Pa.s).  Typical value for water is=1E-3");
  params.addRequiredParam<RealVectorValue>("gravity", "Gravitational acceleration (m/s^2) as a vector pointing downwards.  Eg (0,0,-10)");
  //params.addRequiredCoupledVar("pressure_vars", "List of variables that represent the pressure");
  params.addParam<bool>("linear_shape_fcns", true, "If you are using second-order Lagrange shape functions you need to set this to false.");

  return params;
}

RichardsMaterial::RichardsMaterial(const std::string & name,
                                   InputParameters parameters) :
    Material(name, parameters),

    _material_por(getParam<Real>("mat_porosity")),
    _por_change(isCoupled("por_change") ? &coupledValue("por_change") : &_zero), // coupledValue returns a reference (an alias) to a VariableValue, and the & turns it into a pointer
    _por_change_old(isCoupled("por_change") ? &coupledValueOld("por_change") : &_zero),

    _material_perm(getParam<RealTensorValue>("mat_permeability")),

    _trace_perm(_material_perm.tr()),

    _material_viscosity(getParam<std::vector<Real> >("viscosity")),

    _material_gravity(getParam<RealVectorValue>("gravity")),

    // Declare that this material is going to provide a Real
    // valued property named "porosity", etc, that Kernels can use.
    _porosity_old(declareProperty<Real>("porosity_old")),
    _porosity(declareProperty<Real>("porosity")),
    _permeability(declareProperty<RealTensorValue>("permeability")),
    _gravity(declareProperty<RealVectorValue>("gravity")),

    _richards_name_UO(getUserObject<RichardsVarNames>("richardsVarNames_UO")),
    _num_p(_richards_name_UO.num_v()),

    _pp_old(declareProperty<std::vector<Real> >("porepressure_old")),
    _pp(declareProperty<std::vector<Real> >("porepressure")),
    _dpp_dv(declareProperty<std::vector<std::vector<Real> > >("dporepressure_dv")),

    _viscosity(declareProperty<std::vector<Real> >("viscosity")),

    _density_old(declareProperty<std::vector<Real> >("density_old")),
    _density(declareProperty<std::vector<Real> >("density")),
    _ddensity_dv(declareProperty<std::vector<std::vector<Real> > >("ddensity_dv")),

    _seff_old(declareProperty<std::vector<Real> >("s_eff_old")),
    _seff(declareProperty<std::vector<Real> >("s_eff")),
    _dseff_dv(declareProperty<std::vector<std::vector<Real> > >("ds_eff_dv")),

    _sat_old(declareProperty<std::vector<Real> >("sat_old")),
    _sat(declareProperty<std::vector<Real> >("sat")),
    _dsat_dv(declareProperty<std::vector<std::vector<Real> > >("dsat_dv")),

    _rel_perm(declareProperty<std::vector<Real> >("rel_perm")),
    _drel_perm_dv(declareProperty<std::vector<std::vector<Real> > >("drel_perm_dv")),

    _mass_old(declareProperty<std::vector<Real> >("mass_old")),
    _mass(declareProperty<std::vector<Real> >("mass")),
    _dmass(declareProperty<std::vector<std::vector<Real> > >("dmass")),

    _flux_no_mob(declareProperty<std::vector<RealVectorValue> >("flux_no_mob")),
    _dflux_no_mob_dv(declareProperty<std::vector<std::vector<RealVectorValue> > >("dflux_no_mob_dv")),
    _dflux_no_mob_dgradv(declareProperty<std::vector<std::vector<RealTensorValue> > >("dflux_no_mob_dgradv")),

    _flux(declareProperty<std::vector<RealVectorValue> >("flux")),
    _dflux_dv(declareProperty<std::vector<std::vector<RealVectorValue> > >("dflux_dv")),
    _dflux_dgradv(declareProperty<std::vector<std::vector<RealTensorValue> > >("dflux_dgradv")),
    _d2flux_dvdv(declareProperty<std::vector<std::vector<std::vector<RealVectorValue> > > >("d2flux_dvdv")),
    _d2flux_dgradvdv(declareProperty<std::vector<std::vector<std::vector<RealTensorValue> > > >("d2flux_dgradvdv")),
    _d2flux_dvdgradv(declareProperty<std::vector<std::vector<std::vector<RealTensorValue> > > >("d2flux_dvdgradv")),

    _tauvel_SUPG(declareProperty<std::vector<RealVectorValue> >("tauvel_SUPG")),
    _dtauvel_SUPG_dgradp(declareProperty<std::vector<RealTensorValue> >("dtauvel_SUPG_dgradv")),
    _dtauvel_SUPG_dp(declareProperty<std::vector<RealVectorValue> >("dtauvel_SUPG_dv"))

{

  // Need to add the variables that the user object is coupled to as dependencies so MOOSE will compute them
  {
    const std::vector<MooseVariable *> & coupled_vars = _richards_name_UO.getCoupledMooseVars();
    for (unsigned int i=0; i<coupled_vars.size(); i++)
      addMooseVariableDependency(coupled_vars[i]);
  }

  if (_material_por <= 0 || _material_por >= 1)
    mooseError("Porosity set to " << _material_por << " but it must be between 0 and 1");

  if (isCoupled("perm_change") && (coupledComponents("perm_change") != 9))
    mooseError("9 components of perm_change must be given to a RichardsMaterial.  You supplied " << coupledComponents("perm_change") << "\n");

  _perm_change.resize(9);
  for (unsigned int i=0 ; i<9 ; ++i)
    _perm_change[i] = (isCoupled("perm_change")? &coupledValue("perm_change", i) : &_zero); // coupledValue returns a reference (an alias) to a VariableValue, and the & turns it into a pointer

  _d2density.resize(_num_p);
  _d2seff.resize(_num_p);
  _d2rel_perm_dv.resize(_num_p);
  _pressure_vals.resize(_num_p);
  _pressure_old_vals.resize(_num_p);
  _material_relperm_UO.resize(_num_p);
  _material_seff_UO.resize(_num_p);
  _material_sat_UO.resize(_num_p);
  _material_density_UO.resize(_num_p);
  _material_SUPG_UO.resize(_num_p);
  _grad_p.resize(_num_p);


  for (unsigned int i=0 ; i<_num_p; ++i)
  {
    // DON'T WANT "pressure_vars" at all since pp_name_UO contains the same info
    //_pressure_vals[i] = &coupledValue("pressure_vars", i); // coupled value returns a reference
    //_pressure_old_vals[i] = (_is_transient ? &coupledValueOld("pressure_vars", i) : &_zero);
    //_grad_p[i] = &coupledGradient("pressure_vars", i);

    _pressure_vals[i] = _richards_name_UO.richards_vals(i);
    _pressure_old_vals[i] = _richards_name_UO.richards_vals_old(i);
    _grad_p[i] = _richards_name_UO.grad_var(i);

    // in the following.  first get the userobject names that were inputted, then get the i_th one of these, then get the actual userobject that this corresponds to, then finally & gives pointer to RichardsRelPerm object.
    _material_relperm_UO[i] = &getUserObjectByName<RichardsRelPerm>(getParam<std::vector<UserObjectName> >("relperm_UO")[i]);
    _material_seff_UO[i] = &getUserObjectByName<RichardsSeff>(getParam<std::vector<UserObjectName> >("seff_UO")[i]);
    _material_sat_UO[i] = &getUserObjectByName<RichardsSat>(getParam<std::vector<UserObjectName> >("sat_UO")[i]);
    _material_density_UO[i] = &getUserObjectByName<RichardsDensity>(getParam<std::vector<UserObjectName> >("density_UO")[i]);
    _material_SUPG_UO[i] = &getUserObjectByName<RichardsSUPG>(getParam<std::vector<UserObjectName> >("SUPG_UO")[i]);
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
  // Grab reference to linear Lagrange finite element object pointer,
  // currently this is always a linear Lagrange element, so this might need to
  // be generalized if we start working with higher-order elements...
  FEBase * & fe(_assembly.getFE(getParam<bool>("linear_shape_fcns") ? FEType(FIRST, LAGRANGE) : FEType(SECOND, LAGRANGE), _current_elem->dim()));

  // Grab references to FE object's mapping data from the _subproblem's FE object
  const std::vector<Real>& dxidx(fe->get_dxidx());
  const std::vector<Real>& dxidy(fe->get_dxidy());
  const std::vector<Real>& dxidz(fe->get_dxidz());
  const std::vector<Real>& detadx(fe->get_detadx());
  const std::vector<Real>& detady(fe->get_detady());
  const std::vector<Real>& detadz(fe->get_detadz());
  const std::vector<Real>& dzetadx(fe->get_dzetadx());
  const std::vector<Real>& dzetady(fe->get_dzetady());
  const std::vector<Real>& dzetadz(fe->get_dzetadz());

  // this gets run for each element
  for (unsigned int qp=0; qp<_qrule->n_points(); qp++)
  {

    _porosity[qp] = _material_por + (*_por_change)[qp];
    _porosity_old[qp] = _material_por + (*_por_change_old)[qp];

    _permeability[qp] = _material_perm;
    for (unsigned int i=0; i<3; i++)
      for (unsigned int j=0; j<3; j++)
        _permeability[qp](i,j) *= std::pow(10,(*_perm_change[3*i+j])[qp]);

    _gravity[qp] = _material_gravity;

    _pp_old[qp].resize(_num_p);
    _pp[qp].resize(_num_p);
    _dpp_dv[qp].resize(_num_p);

    _viscosity[qp].resize(_num_p);

    _density_old[qp].resize(_num_p);
    _density[qp].resize(_num_p);
    _ddensity_dv[qp].resize(_num_p);

    _rel_perm[qp].resize(_num_p);
    _drel_perm_dv[qp].resize(_num_p);

    _mass_old[qp].resize(_num_p);
    _mass[qp].resize(_num_p);
    _dmass[qp].resize(_num_p);

    _flux_no_mob[qp].resize(_num_p);
    _dflux_no_mob_dv[qp].resize(_num_p);
    _dflux_no_mob_dgradv[qp].resize(_num_p);

    _flux[qp].resize(_num_p);
    _dflux_dv[qp].resize(_num_p);
    _dflux_dgradv[qp].resize(_num_p);
    _d2flux_dvdv[qp].resize(_num_p);
    _d2flux_dgradvdv[qp].resize(_num_p);
    _d2flux_dvdgradv[qp].resize(_num_p);

    _seff_old[qp].resize(_num_p);
    _seff[qp].resize(_num_p);
    _dseff_dv[qp].resize(_num_p);

    _sat_old[qp].resize(_num_p);
    _sat[qp].resize(_num_p);
    _dsat_dv[qp].resize(_num_p);


    for (unsigned int i=0 ; i<_num_p; ++i)
    {
      _pp_old[qp][i] = (*_pressure_old_vals[i])[qp];
      _pp[qp][i] = (*_pressure_vals[i])[qp];
      _dpp_dv[qp][i].resize(_num_p);
      for (unsigned int j=0 ; j<_num_p; ++j)
        _dpp_dv[qp][i][j] = 0;
      _dpp_dv[qp][i][i] = 1;


      _viscosity[qp][i] = _material_viscosity[i];

      _density_old[qp][i] = (*_material_density_UO[i]).density((*_pressure_old_vals[i])[qp]);
      _density[qp][i] = (*_material_density_UO[i]).density((*_pressure_vals[i])[qp]);
      _ddensity_dv[qp][i].resize(_num_p);
      for (unsigned int j=0 ; j<_num_p; ++j)
        _ddensity_dv[qp][i][j] = 0;
      _ddensity_dv[qp][i][i] = (*_material_density_UO[i]).ddensity((*_pressure_vals[i])[qp]);

      _d2density[i] = (*_material_density_UO[i]).d2density((*_pressure_vals[i])[qp]);

      _seff_old[qp][i] = (*_material_seff_UO[i]).seff(_pressure_old_vals, qp);
      _seff[qp][i] = (*_material_seff_UO[i]).seff(_pressure_vals, qp);

      _dseff_dv[qp][i].resize(_num_p);
      _dseff_dv[qp][i] = (*_material_seff_UO[i]).dseff(_pressure_vals, qp);

      _d2seff[i].resize(_num_p);
      for (unsigned int j=0 ; j<_num_p; ++j)
        _d2seff[i][j].resize(_num_p);
      _d2seff[i] = (*_material_seff_UO[i]).d2seff(_pressure_vals, qp);

      _sat_old[qp][i] = (*_material_sat_UO[i]).sat(_seff_old[qp][i]);
      _sat[qp][i] = (*_material_sat_UO[i]).sat(_seff[qp][i]);

      _dsat_dv[qp][i].resize(_num_p);
      for (unsigned int j=0 ; j<_num_p; ++j)
        _dsat_dv[qp][i][j] = (*_material_sat_UO[i]).dsat(_seff[qp][i])*_dseff_dv[qp][i][j]; // could optimise


      _rel_perm[qp][i] = (*_material_relperm_UO[i]).relperm(_seff[qp][i]);
      _drel_perm_dv[qp][i].resize(_num_p);
      _d2rel_perm_dv[i].resize(_num_p);
      Real drel;
      Real d2rel;
      for (unsigned int j=0 ; j<_num_p; ++j)
      {
        drel = (*_material_relperm_UO[i]).drelperm(_seff[qp][i]);
        d2rel = (* _material_relperm_UO[i]).d2relperm(_seff[qp][i]);
        _drel_perm_dv[qp][i][j] = drel*_dseff_dv[qp][i][j];
        _d2rel_perm_dv[i][j].resize(_num_p);
        for (unsigned int k=0 ; k<_num_p; ++k)
          _d2rel_perm_dv[i][j][k] = d2rel*_dseff_dv[qp][i][j]*_dseff_dv[qp][i][k] + drel*_d2seff[i][j][k];
      }

      _mass[qp][i] = _porosity[qp]*_density[qp][i]*_sat[qp][i];

      _dmass[qp][i].resize(_num_p);
      for (unsigned int j=0 ; j<_num_p; ++j)
        _dmass[qp][i][j] = _porosity[qp]*_density[qp][i]*_dsat_dv[qp][i][j];
      _dmass[qp][i][i] += _porosity[qp]*_ddensity_dv[qp][i][i]*_sat[qp][i];

      _mass_old[qp][i] = _porosity_old[qp]*_density_old[qp][i]*_sat_old[qp][i];


      _flux_no_mob[qp][i] = _permeability[qp]*((*_grad_p[i])[qp] - _density[qp][i]*_gravity[qp]);

      _dflux_no_mob_dv[qp][i].resize(_num_p);
      for (unsigned int j=0 ; j<_num_p; ++j)
        _dflux_no_mob_dv[qp][i][j] = 0.0;
      _dflux_no_mob_dv[qp][i][i] = _permeability[qp]*(- _ddensity_dv[qp][i][i]*_gravity[qp]);

      _dflux_no_mob_dgradv[qp][i].resize(_num_p);
      for (unsigned int j=0 ; j<_num_p; ++j)
        _dflux_no_mob_dgradv[qp][i][j] = 0;
      _dflux_no_mob_dgradv[qp][i][i] += _permeability[qp];


      _flux[qp][i] = _density[qp][i]*_rel_perm[qp][i]*_flux_no_mob[qp][i]/_viscosity[qp][i];

      _dflux_dv[qp][i].resize(_num_p);
      for (unsigned int j=0 ; j<_num_p; ++j)
        _dflux_dv[qp][i][j] = _density[qp][i]*_drel_perm_dv[qp][i][j]*(_permeability[qp]*((*_grad_p[i])[qp] - _density[qp][i]*_gravity[qp]))/_viscosity[qp][i];
      _dflux_dv[qp][i][i] += _ddensity_dv[qp][i][i]*_rel_perm[qp][i]*(_permeability[qp]*((*_grad_p[i])[qp] - _density[qp][i]*_gravity[qp]))/_viscosity[qp][i];
      _dflux_dv[qp][i][i] += _density[qp][i]*_rel_perm[qp][i]*(_permeability[qp]*(- _ddensity_dv[qp][i][i]*_gravity[qp]))/_viscosity[qp][i];


      _dflux_dgradv[qp][i].resize(_num_p);
      for (unsigned int j=0 ; j<_num_p; ++j)
        _dflux_dgradv[qp][i][j] = 0;
      _dflux_dgradv[qp][i][i] += _density[qp][i]*_rel_perm[qp][i]*_permeability[qp]/_viscosity[qp][i];


      _d2flux_dvdv[qp][i].resize(_num_p);
      for (unsigned int j=0 ; j<_num_p; ++j)
      {
        _d2flux_dvdv[qp][i][j].resize(_num_p);
        for (unsigned int k=0 ; k<_num_p; ++k)
          _d2flux_dvdv[qp][i][j][k] = _density[qp][i]*_d2rel_perm_dv[i][j][k]*(_permeability[qp]*((*_grad_p[i])[qp] - _density[qp][i]*_gravity[qp]));
        _d2flux_dvdv[qp][i][j][i] += _ddensity_dv[qp][i][i]*_drel_perm_dv[qp][i][j]*(_permeability[qp]*((*_grad_p[i])[qp] - _density[qp][i]*_gravity[qp]));
        _d2flux_dvdv[qp][i][j][i] += _density[qp][i]*_drel_perm_dv[qp][i][j]*(_permeability[qp]*(- _ddensity_dv[qp][i][i]*_gravity[qp]));
      }
      for (unsigned int j=0 ; j<_num_p; ++j)
        _d2flux_dvdv[qp][i][i][j] += _ddensity_dv[qp][i][i]*_drel_perm_dv[qp][i][j]*(_permeability[qp]*((*_grad_p[i])[qp] - _density[qp][i]*_gravity[qp]));
      _d2flux_dvdv[qp][i][i][i] += _d2density[i]*_rel_perm[qp][i]*(_permeability[qp]*((*_grad_p[i])[qp] - _density[qp][i]*_gravity[qp]));
      _d2flux_dvdv[qp][i][i][i] += _ddensity_dv[qp][i][i]*_rel_perm[qp][i]*(_permeability[qp]*(- _ddensity_dv[qp][i][i]*_gravity[qp]));
      for (unsigned int j=0 ; j<_num_p; ++j)
        _d2flux_dvdv[qp][i][i][j] += _density[qp][i]*_drel_perm_dv[qp][i][j]*(_permeability[qp]*(- _ddensity_dv[qp][i][i]*_gravity[qp]));
      _d2flux_dvdv[qp][i][i][i] += _ddensity_dv[qp][i][i]*_rel_perm[qp][i]*(_permeability[qp]*(- _ddensity_dv[qp][i][i]*_gravity[qp]));
      _d2flux_dvdv[qp][i][i][i] += _density[qp][i]*_rel_perm[qp][i]*(_permeability[qp]*(- _d2density[i]*_gravity[qp]));
      for (unsigned int j=0 ; j<_num_p; ++j)
        for (unsigned int k=0 ; k<_num_p; ++k)
          _d2flux_dvdv[qp][i][j][k] /= _viscosity[qp][i];


      _d2flux_dgradvdv[qp][i].resize(_num_p);
      for (unsigned int j=0 ; j<_num_p; ++j)
      {
        _d2flux_dgradvdv[qp][i][j].resize(_num_p);
        for (unsigned int k=0 ; k<_num_p; ++k)
          _d2flux_dgradvdv[qp][i][j][k] = 0;
      }
      for (unsigned int j=0 ; j<_num_p; ++j)
        _d2flux_dgradvdv[qp][i][i][j] += _density[qp][i]*_drel_perm_dv[qp][i][j]*_permeability[qp]/_viscosity[qp][i];
      _d2flux_dgradvdv[qp][i][i][i] += _ddensity_dv[qp][i][i]*_rel_perm[qp][i]*_permeability[qp]/_viscosity[qp][i];


      _d2flux_dvdgradv[qp][i].resize(_num_p);
      for (unsigned int j=0 ; j<_num_p; ++j)
      {
        _d2flux_dvdgradv[qp][i][j].resize(_num_p);
        for (unsigned int k=0 ; k<_num_p; ++k)
          _d2flux_dvdgradv[qp][i][j][k] = 0;
        _d2flux_dvdgradv[qp][i][j][i] += _density[qp][i]*_drel_perm_dv[qp][i][j]*_permeability[qp]/_viscosity[qp][i];
      }
      _d2flux_dvdgradv[qp][i][i][i] += _ddensity_dv[qp][i][i]*_rel_perm[qp][i]*_permeability[qp]/_viscosity[qp][i];


    }


    // Now SUPG stuff
    _tauvel_SUPG[qp].resize(_num_p);
    _dtauvel_SUPG_dgradp[qp].resize(_num_p);
    _dtauvel_SUPG_dp[qp].resize(_num_p);

    // Bounds checking on element data and putting into vector form
    mooseAssert(qp < dxidx.size(), "Insufficient data in dxidx array!");
    mooseAssert(qp < dxidy.size(), "Insufficient data in dxidy array!");
    mooseAssert(qp < dxidz.size(), "Insufficient data in dxidz array!");
    if (_mesh.dimension() >= 2)
    {
      mooseAssert(qp < detadx.size(), "Insufficient data in detadx array!");
      mooseAssert(qp < detady.size(), "Insufficient data in detady array!");
      mooseAssert(qp < detadz.size(), "Insufficient data in detadz array!");
    }
    if (_mesh.dimension() >= 3)
    {
      mooseAssert(qp < dzetadx.size(), "Insufficient data in dzetadx array!");
      mooseAssert(qp < dzetady.size(), "Insufficient data in dzetady array!");
      mooseAssert(qp < dzetadz.size(), "Insufficient data in dzetadz array!");
    }

    // CHECK : Does this work spherical, cylindrical, etc?
    RealVectorValue xi_prime(dxidx[qp], dxidy[qp], dxidz[qp]);
    RealVectorValue eta_prime, zeta_prime;
    if (_mesh.dimension() >= 2)
    {
      eta_prime(0) = detadx[qp];
      eta_prime(1) = detady[qp];
    }
    if (_mesh.dimension() == 3)
    {
      eta_prime(2) = detadz[qp];
      zeta_prime(0) = dzetadx[qp];
      zeta_prime(1) = dzetady[qp];
      zeta_prime(2) = dzetadz[qp];
    }

    for (unsigned int i=0 ; i<_num_p; ++i)
    {
      RealVectorValue vel = (*_material_SUPG_UO[i]).velSUPG(_permeability[qp], (*_grad_p[i])[qp], _density[qp][i], _gravity[qp]);
      RealTensorValue dvel_dgradp = (*_material_SUPG_UO[i]).dvelSUPG_dgradp(_permeability[qp]);
      RealVectorValue dvel_dp = (*_material_SUPG_UO[i]).dvelSUPG_dp(_permeability[qp], _ddensity_dv[qp][i][i], _gravity[qp]);
      RealVectorValue bb = (*_material_SUPG_UO[i]).bb(vel, _mesh.dimension(), xi_prime, eta_prime, zeta_prime);
      RealVectorValue dbb2_dgradp = (*_material_SUPG_UO[i]).dbb2_dgradp(vel, dvel_dgradp, xi_prime, eta_prime, zeta_prime);
      Real dbb2_dp = (*_material_SUPG_UO[i]).dbb2_dp(vel, dvel_dp, xi_prime, eta_prime, zeta_prime);
      Real tau = (*_material_SUPG_UO[i]).tauSUPG(vel, _trace_perm, bb);
      RealVectorValue dtau_dgradp = (*_material_SUPG_UO[i]).dtauSUPG_dgradp(vel, dvel_dgradp, _trace_perm, bb, dbb2_dgradp);
      Real dtau_dp = (*_material_SUPG_UO[i]).dtauSUPG_dp(vel, dvel_dp, _trace_perm, bb, dbb2_dp);

      _tauvel_SUPG[qp][i] = tau*vel;

      _dtauvel_SUPG_dgradp[qp][i] = tau*dvel_dgradp;
      for (unsigned int j=0 ; j<3; ++j)
        for (unsigned int k=0 ; k<3; ++k)
          _dtauvel_SUPG_dgradp[qp][i](j,k) += dtau_dgradp(j)*vel(k); // this is outerproduct - maybe libmesh can do it better?

      _dtauvel_SUPG_dp[qp][i] = dtau_dp*vel + tau*dvel_dp;
    }
  }
}
