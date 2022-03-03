//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RichardsMaterial.h"
#include "Assembly.h"
#include "MooseMesh.h"

#include <cmath> // std::sinh and std::cosh

#include "libmesh/quadrature.h"

registerMooseObject("RichardsApp", RichardsMaterial);

InputParameters
RichardsMaterial::validParams()
{
  InputParameters params = Material::validParams();

  params.addRequiredParam<Real>(
      "mat_porosity", "The porosity of the material.  Should be between 0 and 1.  Eg, 0.1");
  params.addCoupledVar("por_change",
                       0,
                       "An auxillary variable describing porosity changes.  "
                       "Porosity = mat_porosity + por_change.  If this is not "
                       "provided, zero is used.");
  params.addRequiredParam<RealTensorValue>("mat_permeability", "The permeability tensor (m^2).");
  params.addCoupledVar("perm_change",
                       "A list of auxillary variable describing permeability "
                       "changes.  There must be 9 of these, corresponding to the "
                       "xx, xy, xz, yx, yy, yz, zx, zy, zz components respectively. "
                       " Permeability = mat_permeability*10^(perm_change).");
  params.addRequiredParam<UserObjectName>(
      "richardsVarNames_UO", "The UserObject that holds the list of Richards variable names.");
  params.addRequiredParam<std::vector<UserObjectName>>(
      "relperm_UO", "List of names of user objects that define relative permeability");
  params.addRequiredParam<std::vector<UserObjectName>>(
      "seff_UO",
      "List of name of user objects that define effective saturation as a function of "
      "pressure list");
  params.addRequiredParam<std::vector<UserObjectName>>(
      "sat_UO",
      "List of names of user objects that define saturation as a function of effective saturation");
  params.addRequiredParam<std::vector<UserObjectName>>(
      "density_UO", "List of names of user objects that define the fluid density");
  params.addRequiredParam<std::vector<UserObjectName>>(
      "SUPG_UO", "List of names of user objects that define the SUPG");
  params.addRequiredParam<std::vector<Real>>(
      "viscosity", "List of viscosity of fluids (Pa.s).  Typical value for water is=1E-3");
  params.addRequiredParam<RealVectorValue>(
      "gravity",
      "Gravitational acceleration (m/s^2) as a vector pointing downwards.  Eg (0,0,-10)");
  // params.addRequiredCoupledVar("pressure_vars", "List of variables that represent the pressure");
  params.addParam<bool>(
      "linear_shape_fcns",
      true,
      "If you are using second-order Lagrange shape functions you need to set this to false.");
  return params;
}

RichardsMaterial::RichardsMaterial(const InputParameters & parameters)
  : Material(parameters),

    _material_por(getParam<Real>("mat_porosity")),
    _por_change(coupledValue("por_change")),
    _por_change_old(coupledValueOld("por_change")),

    _material_perm(getParam<RealTensorValue>("mat_permeability")),

    _material_gravity(getParam<RealVectorValue>("gravity")),

    _porosity_old(declareProperty<Real>("porosity_old")),
    _porosity(declareProperty<Real>("porosity")),
    _permeability(declareProperty<RealTensorValue>("permeability")),
    _gravity(declareProperty<RealVectorValue>("gravity")),

    _richards_name_UO(getUserObject<RichardsVarNames>("richardsVarNames_UO")),
    _num_p(_richards_name_UO.num_v()),

    _perm_change(isCoupled("perm_change")
                     ? coupledValues("perm_change")
                     : std::vector<const VariableValue *>(LIBMESH_DIM * LIBMESH_DIM, &_zero)),

    _trace_perm(_material_perm.tr()),

    _material_viscosity(getParam<std::vector<Real>>("viscosity")),

    _pp_old(declareProperty<std::vector<Real>>("porepressure_old")),
    _pp(declareProperty<std::vector<Real>>("porepressure")),
    _dpp_dv(declareProperty<std::vector<std::vector<Real>>>("dporepressure_dv")),
    _d2pp_dv(declareProperty<std::vector<std::vector<std::vector<Real>>>>("d2porepressure_dvdv")),

    _viscosity(declareProperty<std::vector<Real>>("viscosity")),

    _density_old(declareProperty<std::vector<Real>>("density_old")),
    _density(declareProperty<std::vector<Real>>("density")),
    _ddensity_dv(declareProperty<std::vector<std::vector<Real>>>("ddensity_dv")),

    _seff_old(declareProperty<std::vector<Real>>("s_eff_old")),
    _seff(declareProperty<std::vector<Real>>("s_eff")),
    _dseff_dv(declareProperty<std::vector<std::vector<Real>>>("ds_eff_dv")),
    _d2seff_dv(declareProperty<std::vector<std::vector<std::vector<Real>>>>("d2s_eff_dvdv")),

    _sat_old(declareProperty<std::vector<Real>>("sat_old")),
    _sat(declareProperty<std::vector<Real>>("sat")),
    _dsat_dv(declareProperty<std::vector<std::vector<Real>>>("dsat_dv")),

    _rel_perm(declareProperty<std::vector<Real>>("rel_perm")),
    _drel_perm_dv(declareProperty<std::vector<std::vector<Real>>>("drel_perm_dv")),

    _mass_old(declareProperty<std::vector<Real>>("mass_old")),
    _mass(declareProperty<std::vector<Real>>("mass")),
    _dmass(declareProperty<std::vector<std::vector<Real>>>("dmass")),

    _flux_no_mob(declareProperty<std::vector<RealVectorValue>>("flux_no_mob")),
    _dflux_no_mob_dv(declareProperty<std::vector<std::vector<RealVectorValue>>>("dflux_no_mob_dv")),
    _dflux_no_mob_dgradv(
        declareProperty<std::vector<std::vector<RealTensorValue>>>("dflux_no_mob_dgradv")),

    _flux(declareProperty<std::vector<RealVectorValue>>("flux")),
    _dflux_dv(declareProperty<std::vector<std::vector<RealVectorValue>>>("dflux_dv")),
    _dflux_dgradv(declareProperty<std::vector<std::vector<RealTensorValue>>>("dflux_dgradv")),
    _d2flux_dvdv(
        declareProperty<std::vector<std::vector<std::vector<RealVectorValue>>>>("d2flux_dvdv")),
    _d2flux_dgradvdv(
        declareProperty<std::vector<std::vector<std::vector<RealTensorValue>>>>("d2flux_dgradvdv")),
    _d2flux_dvdgradv(
        declareProperty<std::vector<std::vector<std::vector<RealTensorValue>>>>("d2flux_dvdgradv")),

    _tauvel_SUPG(declareProperty<std::vector<RealVectorValue>>("tauvel_SUPG")),
    _dtauvel_SUPG_dgradp(
        declareProperty<std::vector<std::vector<RealTensorValue>>>("dtauvel_SUPG_dgradv")),
    _dtauvel_SUPG_dp(declareProperty<std::vector<std::vector<RealVectorValue>>>("dtauvel_SUPG_dv"))

{

  // Need to add the variables that the user object is coupled to as dependencies so MOOSE will
  // compute them
  {
    const std::vector<MooseVariableFEBase *> & coupled_vars =
        _richards_name_UO.getCoupledMooseVars();
    for (unsigned int i = 0; i < coupled_vars.size(); i++)
      addMooseVariableDependency(coupled_vars[i]);
  }

  if (_material_por <= 0 || _material_por >= 1)
    mooseError("Porosity set to ", _material_por, " but it must be between 0 and 1");

  if (isCoupled("perm_change") && (coupledComponents("perm_change") != LIBMESH_DIM * LIBMESH_DIM))
    mooseError(LIBMESH_DIM * LIBMESH_DIM,
               " components of perm_change must be given to a RichardsMaterial.  You supplied ",
               coupledComponents("perm_change"),
               "\n");

  if (!(_material_viscosity.size() == _num_p &&
        getParam<std::vector<UserObjectName>>("relperm_UO").size() == _num_p &&
        getParam<std::vector<UserObjectName>>("seff_UO").size() == _num_p &&
        getParam<std::vector<UserObjectName>>("sat_UO").size() == _num_p &&
        getParam<std::vector<UserObjectName>>("density_UO").size() == _num_p &&
        getParam<std::vector<UserObjectName>>("SUPG_UO").size() == _num_p))
    mooseError("There are ",
               _num_p,
               " Richards fluid variables, so you need to specify this "
               "number of viscosities, relperm_UO, seff_UO, sat_UO, "
               "density_UO, SUPG_UO");

  _d2density.resize(_num_p);
  _d2rel_perm_dv.resize(_num_p);
  _pressure_vals.resize(_num_p);
  _pressure_old_vals.resize(_num_p);
  _material_relperm_UO.resize(_num_p);
  _material_seff_UO.resize(_num_p);
  _material_sat_UO.resize(_num_p);
  _material_density_UO.resize(_num_p);
  _material_SUPG_UO.resize(_num_p);
  _grad_p.resize(_num_p);

  for (unsigned int i = 0; i < _num_p; ++i)
  {
    // DON'T WANT "pressure_vars" at all since pp_name_UO contains the same info
    //_pressure_vals[i] = &coupledValue("pressure_vars", i); // coupled value returns a reference
    //_pressure_old_vals[i] = (_is_transient ? &coupledValueOld("pressure_vars", i) : &_zero);
    //_grad_p[i] = &coupledGradient("pressure_vars", i);

    // in the following.  first get the userobject names that were inputted, then get the i_th one
    // of these, then get the actual userobject that this corresponds to, then finally & gives
    // pointer to RichardsRelPerm object.
    _material_relperm_UO[i] = &getUserObjectByName<RichardsRelPerm>(
        getParam<std::vector<UserObjectName>>("relperm_UO")[i]);
    _material_seff_UO[i] =
        &getUserObjectByName<RichardsSeff>(getParam<std::vector<UserObjectName>>("seff_UO")[i]);
    _material_sat_UO[i] =
        &getUserObjectByName<RichardsSat>(getParam<std::vector<UserObjectName>>("sat_UO")[i]);
    _material_density_UO[i] = &getUserObjectByName<RichardsDensity>(
        getParam<std::vector<UserObjectName>>("density_UO")[i]);
    _material_SUPG_UO[i] =
        &getUserObjectByName<RichardsSUPG>(getParam<std::vector<UserObjectName>>("SUPG_UO")[i]);
  }
}

void
RichardsMaterial::computePandSeff()
{
  // Get the pressure and effective saturation at each quadpoint
  // From these we will build the relative permeability, density, flux, etc
  if (_richards_name_UO.var_types() == "pppp")
  {
    for (unsigned int i = 0; i < _num_p; ++i)
    {
      _pressure_vals[i] = _richards_name_UO.richards_vals(i);
      _pressure_old_vals[i] = _richards_name_UO.richards_vals_old(i);
      _grad_p[i] = _richards_name_UO.grad_var(i);
    }
  }

  for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
  {
    _pp_old[qp].resize(_num_p);
    _pp[qp].resize(_num_p);
    _dpp_dv[qp].resize(_num_p);
    _d2pp_dv[qp].resize(_num_p);

    _seff_old[qp].resize(_num_p);
    _seff[qp].resize(_num_p);
    _dseff_dv[qp].resize(_num_p);
    _d2seff_dv[qp].resize(_num_p);

    if (_richards_name_UO.var_types() == "pppp")
    {
      for (unsigned int i = 0; i < _num_p; ++i)
      {
        _pp_old[qp][i] = (*_pressure_old_vals[i])[qp];
        _pp[qp][i] = (*_pressure_vals[i])[qp];

        _dpp_dv[qp][i].assign(_num_p, 0);
        _dpp_dv[qp][i][i] = 1;

        _d2pp_dv[qp][i].resize(_num_p);
        for (unsigned int j = 0; j < _num_p; ++j)
          _d2pp_dv[qp][i][j].assign(_num_p, 0);

        _seff_old[qp][i] = (*_material_seff_UO[i]).seff(_pressure_old_vals, qp);
        _seff[qp][i] = (*_material_seff_UO[i]).seff(_pressure_vals, qp);

        _dseff_dv[qp][i].resize(_num_p);
        (*_material_seff_UO[i]).dseff(_pressure_vals, qp, _dseff_dv[qp][i]);

        _d2seff_dv[qp][i].resize(_num_p);
        for (unsigned int j = 0; j < _num_p; ++j)
          _d2seff_dv[qp][i][j].resize(_num_p);
        (*_material_seff_UO[i]).d2seff(_pressure_vals, qp, _d2seff_dv[qp][i]);
      }
    }
    // the above lines of code are only valid for "pppp"
    // if you decide to code other RichardsVariables (eg "psss")
    // you will need to add some lines here
  }
}

void
RichardsMaterial::computeDerivedQuantities(unsigned int qp)
{
  // fluid viscosity
  _viscosity[qp].resize(_num_p);
  for (unsigned int i = 0; i < _num_p; ++i)
    _viscosity[qp][i] = _material_viscosity[i];

  // fluid saturation
  _sat_old[qp].resize(_num_p);
  _sat[qp].resize(_num_p);
  _dsat_dv[qp].resize(_num_p);
  for (unsigned int i = 0; i < _num_p; ++i)
  {
    _sat_old[qp][i] = (*_material_sat_UO[i]).sat(_seff_old[qp][i]);
    _sat[qp][i] = (*_material_sat_UO[i]).sat(_seff[qp][i]);
    _dsat_dv[qp][i].assign(_num_p, (*_material_sat_UO[i]).dsat(_seff[qp][i]));
    for (unsigned int j = 0; j < _num_p; ++j)
      _dsat_dv[qp][i][j] *= _dseff_dv[qp][i][j];
  }

  // fluid density
  _density_old[qp].resize(_num_p);
  _density[qp].resize(_num_p);
  _ddensity_dv[qp].resize(_num_p);
  for (unsigned int i = 0; i < _num_p; ++i)
  {
    _density_old[qp][i] = (*_material_density_UO[i]).density(_pp_old[qp][i]);
    _density[qp][i] = (*_material_density_UO[i]).density(_pp[qp][i]);
    _ddensity_dv[qp][i].assign(_num_p, (*_material_density_UO[i]).ddensity(_pp[qp][i]));
    for (unsigned int j = 0; j < _num_p; ++j)
      _ddensity_dv[qp][i][j] *= _dpp_dv[qp][i][j];
  }

  // relative permeability
  _rel_perm[qp].resize(_num_p);
  _drel_perm_dv[qp].resize(_num_p);
  for (unsigned int i = 0; i < _num_p; ++i)
  {
    _rel_perm[qp][i] = (*_material_relperm_UO[i]).relperm(_seff[qp][i]);
    _drel_perm_dv[qp][i].assign(_num_p, (*_material_relperm_UO[i]).drelperm(_seff[qp][i]));
    for (unsigned int j = 0; j < _num_p; ++j)
      _drel_perm_dv[qp][i][j] *= _dseff_dv[qp][i][j];
  }

  // fluid mass
  _mass_old[qp].resize(_num_p);
  _mass[qp].resize(_num_p);
  _dmass[qp].resize(_num_p);
  for (unsigned int i = 0; i < _num_p; ++i)
  {
    _mass_old[qp][i] = _porosity_old[qp] * _density_old[qp][i] * _sat_old[qp][i];
    _mass[qp][i] = _porosity[qp] * _density[qp][i] * _sat[qp][i];
    _dmass[qp][i].resize(_num_p);
    for (unsigned int j = 0; j < _num_p; ++j)
      _dmass[qp][i][j] = _porosity[qp] * (_ddensity_dv[qp][i][j] * _sat[qp][i] +
                                          _density[qp][i] * _dsat_dv[qp][i][j]);
  }

  // flux without the mobility part
  _flux_no_mob[qp].resize(_num_p);
  _dflux_no_mob_dv[qp].resize(_num_p);
  _dflux_no_mob_dgradv[qp].resize(_num_p);
  for (unsigned int i = 0; i < _num_p; ++i)
  {
    _flux_no_mob[qp][i] = _permeability[qp] * ((*_grad_p[i])[qp] - _density[qp][i] * _gravity[qp]);

    _dflux_no_mob_dv[qp][i].resize(_num_p);
    for (unsigned int j = 0; j < _num_p; ++j)
      _dflux_no_mob_dv[qp][i][j] = _permeability[qp] * (-_ddensity_dv[qp][i][j] * _gravity[qp]);

    _dflux_no_mob_dgradv[qp][i].resize(_num_p);
    for (unsigned int j = 0; j < _num_p; ++j)
      _dflux_no_mob_dgradv[qp][i][j] = _permeability[qp] * _dpp_dv[qp][i][j];
  }

  // flux
  _flux[qp].resize(_num_p);
  _dflux_dv[qp].resize(_num_p);
  _dflux_dgradv[qp].resize(_num_p);
  for (unsigned int i = 0; i < _num_p; ++i)
  {
    _flux[qp][i] = _density[qp][i] * _rel_perm[qp][i] * _flux_no_mob[qp][i] / _viscosity[qp][i];

    _dflux_dv[qp][i].resize(_num_p);
    for (unsigned int j = 0; j < _num_p; ++j)
    {
      _dflux_dv[qp][i][j] =
          _density[qp][i] * _rel_perm[qp][i] * _dflux_no_mob_dv[qp][i][j] / _viscosity[qp][i];
      _dflux_dv[qp][i][j] +=
          (_ddensity_dv[qp][i][j] * _rel_perm[qp][i] + _density[qp][i] * _drel_perm_dv[qp][i][j]) *
          _flux_no_mob[qp][i] / _viscosity[qp][i];
    }

    _dflux_dgradv[qp][i].resize(_num_p);
    for (unsigned int j = 0; j < _num_p; ++j)
      _dflux_dgradv[qp][i][j] =
          _density[qp][i] * _rel_perm[qp][i] * _dflux_no_mob_dgradv[qp][i][j] / _viscosity[qp][i];
  }
}

void
RichardsMaterial::zero2ndDerivedQuantities(unsigned int qp)
{
  _d2flux_dvdv[qp].resize(_num_p);
  _d2flux_dgradvdv[qp].resize(_num_p);
  _d2flux_dvdgradv[qp].resize(_num_p);

  for (unsigned int i = 0; i < _num_p; ++i)
  {
    _d2flux_dvdv[qp][i].resize(_num_p);
    _d2flux_dgradvdv[qp][i].resize(_num_p);
    _d2flux_dvdgradv[qp][i].resize(_num_p);
    for (unsigned int j = 0; j < _num_p; ++j)
    {
      _d2flux_dvdv[qp][i][j].assign(_num_p, RealVectorValue());
      _d2flux_dgradvdv[qp][i][j].assign(_num_p, RealTensorValue());
      _d2flux_dvdgradv[qp][i][j].assign(_num_p, RealTensorValue());
    }
  }
}

void
RichardsMaterial::compute2ndDerivedQuantities(unsigned int qp)
{
  zero2ndDerivedQuantities(qp);

  for (unsigned int i = 0; i < _num_p; ++i)
  {
    if ((*_material_SUPG_UO[i]).SUPG_trivial())
      continue; // as the derivatives won't be needed

    // second derivative of density
    _d2density[i].resize(_num_p);
    Real ddens = (*_material_density_UO[i]).ddensity(_pp[qp][i]);
    Real d2dens = (*_material_density_UO[i]).d2density(_pp[qp][i]);
    for (unsigned int j = 0; j < _num_p; ++j)
    {
      _d2density[i][j].resize(_num_p);
      for (unsigned int k = 0; k < _num_p; ++k)
        _d2density[i][j][k] =
            d2dens * _dpp_dv[qp][i][j] * _dpp_dv[qp][i][k] + ddens * _d2pp_dv[qp][i][j][k];
    }

    // second derivative of relative permeability
    _d2rel_perm_dv[i].resize(_num_p);
    Real drel = (*_material_relperm_UO[i]).drelperm(_seff[qp][i]);
    Real d2rel = (*_material_relperm_UO[i]).d2relperm(_seff[qp][i]);
    for (unsigned int j = 0; j < _num_p; ++j)
    {
      _d2rel_perm_dv[i][j].resize(_num_p);
      for (unsigned int k = 0; k < _num_p; ++k)
        _d2rel_perm_dv[i][j][k] =
            d2rel * _dseff_dv[qp][i][j] * _dseff_dv[qp][i][k] + drel * _d2seff_dv[qp][i][j][k];
    }

    // now compute the second derivs of the fluxes
    for (unsigned int j = 0; j < _num_p; ++j)
    {
      for (unsigned int k = 0; k < _num_p; ++k)
      {
        _d2flux_dvdv[qp][i][j][k] =
            _d2density[i][j][k] * _rel_perm[qp][i] *
            (_permeability[qp] * ((*_grad_p[i])[qp] - _density[qp][i] * _gravity[qp]));
        _d2flux_dvdv[qp][i][j][k] +=
            (_ddensity_dv[qp][i][j] * _drel_perm_dv[qp][i][k] +
             _ddensity_dv[qp][i][k] * _drel_perm_dv[qp][i][j]) *
            (_permeability[qp] * ((*_grad_p[i])[qp] - _density[qp][i] * _gravity[qp]));
        _d2flux_dvdv[qp][i][j][k] +=
            _density[qp][i] * _d2rel_perm_dv[i][j][k] *
            (_permeability[qp] * ((*_grad_p[i])[qp] - _density[qp][i] * _gravity[qp]));
        _d2flux_dvdv[qp][i][j][k] += (_ddensity_dv[qp][i][j] * _rel_perm[qp][i] +
                                      _density[qp][i] * _drel_perm_dv[qp][i][j]) *
                                     (_permeability[qp] * (-_ddensity_dv[qp][i][k] * _gravity[qp]));
        _d2flux_dvdv[qp][i][j][k] += (_ddensity_dv[qp][i][k] * _rel_perm[qp][i] +
                                      _density[qp][i] * _drel_perm_dv[qp][i][k]) *
                                     (_permeability[qp] * (-_ddensity_dv[qp][i][j] * _gravity[qp]));
        _d2flux_dvdv[qp][i][j][k] += _density[qp][i] * _rel_perm[qp][i] *
                                     (_permeability[qp] * (-_d2density[i][j][k] * _gravity[qp]));
      }
    }
    for (unsigned int j = 0; j < _num_p; ++j)
      for (unsigned int k = 0; k < _num_p; ++k)
        _d2flux_dvdv[qp][i][j][k] /= _viscosity[qp][i];

    for (unsigned int j = 0; j < _num_p; ++j)
    {
      for (unsigned int k = 0; k < _num_p; ++k)
      {
        _d2flux_dgradvdv[qp][i][j][k] = (_ddensity_dv[qp][i][k] * _rel_perm[qp][i] +
                                         _density[qp][i] * _drel_perm_dv[qp][i][k]) *
                                        _permeability[qp] * _dpp_dv[qp][i][j] / _viscosity[qp][i];
        _d2flux_dvdgradv[qp][i][k][j] = _d2flux_dgradvdv[qp][i][j][k];
      }
    }
  }
}

void
RichardsMaterial::zeroSUPG(unsigned int qp)
{
  _tauvel_SUPG[qp].assign(_num_p, RealVectorValue());
  _dtauvel_SUPG_dgradp[qp].resize(_num_p);
  _dtauvel_SUPG_dp[qp].resize(_num_p);
  for (unsigned int i = 0; i < _num_p; ++i)
  {
    _dtauvel_SUPG_dp[qp][i].assign(_num_p, RealVectorValue());
    _dtauvel_SUPG_dgradp[qp][i].assign(_num_p, RealTensorValue());
  }
}

void
RichardsMaterial::computeSUPG()
{
  // Grab reference to linear Lagrange finite element object pointer,
  // currently this is always a linear Lagrange element, so this might need to
  // be generalized if we start working with higher-order elements...
  auto && fe = _assembly.getFE(getParam<bool>("linear_shape_fcns") ? FEType(FIRST, LAGRANGE)
                                                                   : FEType(SECOND, LAGRANGE),
                               _current_elem->dim());

  // Grab references to FE object's mapping data from the _subproblem's FE object
  const std::vector<Real> & dxidx(fe->get_dxidx());
  const std::vector<Real> & dxidy(fe->get_dxidy());
  const std::vector<Real> & dxidz(fe->get_dxidz());
  const std::vector<Real> & detadx(fe->get_detadx());
  const std::vector<Real> & detady(fe->get_detady());
  const std::vector<Real> & detadz(fe->get_detadz());
  const std::vector<Real> & dzetadx(fe->get_dzetadx());
  const std::vector<Real> & dzetady(fe->get_dzetady());
  const std::vector<Real> & dzetadz(fe->get_dzetadz());

  for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
  {

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

    _trace_perm = _permeability[qp].tr();
    for (unsigned int i = 0; i < _num_p; ++i)
    {
      RealVectorValue vel =
          (*_material_SUPG_UO[i])
              .velSUPG(_permeability[qp], (*_grad_p[i])[qp], _density[qp][i], _gravity[qp]);
      RealTensorValue dvel_dgradp = (*_material_SUPG_UO[i]).dvelSUPG_dgradp(_permeability[qp]);
      RealVectorValue dvel_dp =
          (*_material_SUPG_UO[i])
              .dvelSUPG_dp(_permeability[qp], _ddensity_dv[qp][i][i], _gravity[qp]);
      RealVectorValue bb =
          (*_material_SUPG_UO[i]).bb(vel, _mesh.dimension(), xi_prime, eta_prime, zeta_prime);
      RealVectorValue dbb2_dgradp =
          (*_material_SUPG_UO[i]).dbb2_dgradp(vel, dvel_dgradp, xi_prime, eta_prime, zeta_prime);
      Real dbb2_dp = (*_material_SUPG_UO[i]).dbb2_dp(vel, dvel_dp, xi_prime, eta_prime, zeta_prime);
      Real tau = (*_material_SUPG_UO[i]).tauSUPG(vel, _trace_perm, bb);
      RealVectorValue dtau_dgradp =
          (*_material_SUPG_UO[i]).dtauSUPG_dgradp(vel, dvel_dgradp, _trace_perm, bb, dbb2_dgradp);
      Real dtau_dp = (*_material_SUPG_UO[i]).dtauSUPG_dp(vel, dvel_dp, _trace_perm, bb, dbb2_dp);

      _tauvel_SUPG[qp][i] = tau * vel;

      RealTensorValue dtauvel_dgradp = tau * dvel_dgradp;
      for (const auto j : make_range(Moose::dim))
        for (const auto k : make_range(Moose::dim))
          dtauvel_dgradp(j, k) +=
              dtau_dgradp(j) * vel(k); // this is outerproduct - maybe libmesh can do it better?
      for (unsigned int j = 0; j < _num_p; ++j)
        _dtauvel_SUPG_dgradp[qp][i][j] = dtauvel_dgradp * _dpp_dv[qp][i][j];

      RealVectorValue dtauvel_dp = dtau_dp * vel + tau * dvel_dp;
      for (unsigned int j = 0; j < _num_p; ++j)
        _dtauvel_SUPG_dp[qp][i][j] = dtauvel_dp * _dpp_dv[qp][i][j];
    }
  }
}

void
RichardsMaterial::computeProperties()
{
  // compute porepressures and effective saturations
  // with algorithms depending on the _richards_name_UO.var_types()
  computePandSeff();

  // porosity, permeability, and gravity
  for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
  {
    _porosity[qp] = _material_por + _por_change[qp];
    _porosity_old[qp] = _material_por + _por_change_old[qp];

    _permeability[qp] = _material_perm;
    for (const auto i : make_range(Moose::dim))
      for (const auto j : make_range(Moose::dim))
        _permeability[qp](i, j) *= std::pow(10, (*_perm_change[LIBMESH_DIM * i + j])[qp]);

    _gravity[qp] = _material_gravity;
  }

  // compute "derived" quantities -- those that depend on P and Seff --- such as density, relperm
  for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
    computeDerivedQuantities(qp);

  // compute certain second derivatives of the derived quantities
  // These are needed in Jacobian calculations if doing SUPG
  for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
    compute2ndDerivedQuantities(qp);

  // Now for SUPG itself
  for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
    zeroSUPG(qp);

  // the following saves computational effort if all SUPG is trivial
  bool trivial_supg = true;
  for (unsigned int i = 0; i < _num_p; ++i)
    trivial_supg = trivial_supg && (*_material_SUPG_UO[i]).SUPG_trivial();
  if (trivial_supg)
    return;
  else
    computeSUPG();
}
