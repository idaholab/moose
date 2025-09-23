/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

#include "LiquidMetalCDAP.h"
#include "AuxiliarySystem.h"
#include "TriSubChannelMesh.h"
#include <array>
#include <cmath>
#include <algorithm>
#include <vector>
#include <numeric>   // std::accumulate
#include <iostream>  // std::cout   (or remove the debug prints)


registerMooseObject("SubChannelApp", LiquidMetalCDAP);

InputParameters
LiquidMetalCDAP::validParams()
{
  InputParameters params = PinTempSolver::validParams();
  params.addClassDescription("Solver class for Clad Damage Propagation for SFR metal fuels in a triangular lattice "
                             "assembly and bare/wire-wrapped fuel rods");

  params.addRequiredParam<Real>("normalized_min_radius", "Minimum radius of two phase exposure");
  params.addRequiredParam<Real>("breach_size", "fuel breach size");
  params.addRequiredParam<Real>("breach_diameter_ref", "Reference breach diameter");
  params.addRequiredParam<Real>("plenum_gas_to_coolant_ratio_ref", "Reference plenum gas to coolant ratio");
  params.addRequiredParam<Real>("two_phase_radius_oil", "Two-phase radius at breach plane in oil");
  params.addRequiredParam<Real>("two_phase_slope_oil", "Two-phase expansion slope in oil");
  params.addRequiredParam<Real>("max_two_phase_radius", "Maximum allowed radius for two phase expansion");
  params.addRequiredParam<Real>("initial_plenum_pressure", "Initial or reference plenum pressure");
  params.addRequiredParam<Real>("initial_plenum_temperature", "Initial plenum gas temperature");
  params.addRequiredParam<Real>("plenum_to_fuel_ratio", "Plenum to fuel volume ratio");
  params.addRequiredParam<Real>("upper_limit_cdf", "Upper limit of cladding CDF");
  params.addRequiredParam<Real>("molar_gas_mass", "Molar plenum gas mass");
  params.addRequiredParam<Real>("jet_length", "Fission gas impinging jet length");
  params.addRequiredParam<Real>("mean_of_cdf", "Mean of CDF for normal dist");
  params.addRequiredParam<Real>("sigma_of_cdf", "Sigma of CDf for normal dist");

//Real _radius_frac            = 0.20;    // “radius frac” in OCR
//Real _deqFail                = 1.0e-3;     // DeqFail
//std::vector<Real> _p_ratio; //
//Real _breach_diameter_ref    = 1.0e-3;
//Real _pgas_to_coolant_ratio_ref = 50.0;

//Real _r_oil = 0.02;     // m
//Real _m_oil = 0.2;       // slope [m/m]
//Real _max_two_phase_radius = 1.0; // m

    /// initial plenum pressure
 //   Real _plegas0 = 20.0e+6;
    /// initial plenum temperature
  //  Real _Tplegas0 = 800.0;

//    Real _ptof_ratio = 1.6;
    
  //    Real _cdfuplim = 10.0;
 //  Real _mgas = 39.948e-03;
  //   Real _jetLength = 1.42e-3;
     
   //     Real _cdfmean = 0.0;
    /// sigma of failure probability/CDF normal distribution
  //  Real _cdfsigma = 1.0;
     
  return params;
}

LiquidMetalCDAP::LiquidMetalCDAP(
    const InputParameters & params)
  : PinTempSolver(params),
    _tri_sch_mesh(dynamic_cast<TriSubChannelMesh &>(_subchannel_mesh)),
    _radius_frac(getParam<Real>("normalized_min_radius")),
    _deqFail(getParam<Real>("breach_size")),
    _breach_diameter_ref(getParam<Real>("breach_diameter_ref")),
    _pgas_to_coolant_ratio_ref(getParam<Real>("plenum_gas_to_coolant_ratio_ref")),
    _r_oil(getParam<Real>("two_phase_radius_oil")),
    _m_oil(getParam<Real>("two_phase_slope_oil")),
    _max_two_phase_radius(getParam<Real>("max_two_phase_radius")),
    _plegas0(getParam<Real>("initial_plenum_pressure")),
    _Tplegas0(getParam<Real>("initial_plenum_temperature")),
    _ptof_ratio(getParam<Real>("plenum_to_fuel_ratio")),
    _cdfuplim(getParam<Real>("upper_limit_cdf")),
    _mgas(getParam<Real>("molar_gas_mass")),
    _jetLength(getParam<Real>("jet_length")),
    _cdfmean(getParam<Real>("mean_of_cdf")),
    _cdfsigma(getParam<Real>("sigma_of_cdf"))
{

}

LiquidMetalCDAP::~LiquidMetalCDAP()
{

}

void
LiquidMetalCDAP::initializeSolutionCDAP()
{

  Real fuel_length = _subchannel_mesh.getHeatedLength();
  

  // Read geometry from base solver via getters
  _r0  = r0();
  _rfu = rfu();      // 'this->' optional
  _rci = rci();
  _nrfuel = nrfuel();
  


  _total_prob = 0.0;
  _total_survivor_prob = 0.0;
  _total_fail = 0.0;

  _clad_cdf.setZero(_n_cells+1, _n_pins);
  _clad_cdf_old.setZero(_n_cells+1, _n_pins);

  _clad_trup.setZero(_n_cells+1, _n_pins);

  _clad_wastage_in.setZero(_n_cells+1, _n_pins);
  _clad_wastage_out.setZero(_n_cells+1, _n_pins);
  _clad_wastage_in_old.setZero(_n_cells+1, _n_pins);
  _clad_wastage_out_old.setZero(_n_cells+1, _n_pins);
  _stress.setZero(_n_cells+1, _n_pins);

  _ifail.resize(_n_pins, 0);
  _pgas.resize(_n_pins, 0.0);
 
  _fail_prob.resize(_n_pins, 0.0);
  _peak_cdf.resize(_n_pins, 0.0);

  _neig_chan.resize(_n_pins, 0);
  _chan_frac.resize(_n_pins, 0);
  _chan_fail.resize(_n_pins, 0);
  _time_ijet.resize(_n_pins, 0);


  _mfg.resize(_n_pins, 0);
  _mdote.resize(_n_pins, 0);

  _pst_crit.resize(_n_pins, 0);
  _pst.resize(_n_pins, 0);
  
  _alpha_intense.resize(_n_cells+1,0.0);
  _HTFG_IntenseChan.setOnes(_n_cells+1,_n_channels) ;
 
  _inodeFG_Intense.setZero(_n_cells+1,_n_channels) ;

  _mdot_ratio = 1.0;
  _HT_FG.resize(_n_cells+1, 1);
  _p_ratio.resize(_n_pins, 30);
  _z_FailFG.resize(_n_pins, 0.0);
  _lengthz_FailFG.resize(_n_pins, 0.0);
  _alpha = 0.5; //initial guess
  
  _mdotfg_tot = 0.0;
  _mdotFG_chan_ave = 0.0;
  _lowz_boundFG = 0.0;
  _highz_boundFG = 0.0;
  
  _rhoFG.resize(_n_pins, 0.0);
  _timestep_counterFG = 0;
  int stepsFG = 10000;
  _trailFG.resize(stepsFG, 0.0);
  _mdotFG_ej.resize(stepsFG,0.0);
  _inodeFG.resize(_n_cells+1, 0);
  
  Real fuvol0 = libMesh::pi*(pow(_rfu,2) - pow(_r0,2))*fuel_length;
  _plenum_vol = _ptof_ratio*fuvol0;


  // allocate & zero
  _dz.assign(_n_cells + 1, 0.0);
  _zloc.assign(_n_cells + 1, 0.0);

  // convention: zloc[0] = 0 (relative coordinates). If you want absolute coords, set _zloc[0] = _z0.
  _zloc[0] = 0.0;

  // fill per-cell thickness and cumulative edges
  for (unsigned int j = 1; j <= static_cast<unsigned int>(_n_cells); ++j)
 {
  _dz[j]   = _z_grid[j] - _z_grid[j - 1];  // thickness of cell j (between edges j-1 and j)
  _zloc[j] = _zloc[j - 1] + _dz[j];        // cumulative edge position
 }




}


/// HT9 time to rupture due to thermal creep - Steady State CDF Model
Real LiquidMetalCDAP::trups1(Real stress, Real Tclad)
{
  Real trup1 = 3600.0*pow(10.0,(-32.49+57781.0/Tclad-11800.0/Tclad*log10(stress)));
  Real trup2 = 3600.0*pow(10.0,(-35.173+45858.0/Tclad-5563.0/Tclad*log10(stress)));
  return std::min(trup1, trup2);
}

/// HT9 time to rupture dut to thermal creep - Transient CDF Model
Real LiquidMetalCDAP::trupt1(Real tcdot, Real stress, Real Tclad)
{
  Real const sigstr = 730.0;
  Real const qht9 = 70170.0/1.987;
  Real DX0 = tanh(2.0E-02*(stress-200.0));
  Real DX1 = tanh((tcdot-58.0)/17.0);
  Real DX2=(-0.5*(1.0 +DX0))*(0.75*(1.0+DX1));
  Real AX2 = -34.8 + DX0 + DX2;
  Real BX2 = 12.0 / (1.5 + 0.5*DX0);
  Real WX2 = AX2+BX2*log(log(sigstr/stress));
  return exp(WX2)*exp(qht9/Tclad);
}

/// HT9 time to rupture due to thermal creep - Combined CDF Model
Real LiquidMetalCDAP::trupht9(Real tcdot, Real stress, Real Tclad)
{
  Real trup1 = 0.0;
  Real trup2 = 0.0;

  Real const Tlow = 973.15;
  Real const Thigh = 1042.15;

  Real result = 0.0;

  if (Tclad <= Tlow) {

    Real sigs1 = 670.0-0.70*(Tclad-273.15);
    Real sigt1 = 1370.0-1.7*(Tclad-273.15);

    if(stress <= sigs1) {

      result = trups1(stress, Tclad);

    } else if(stress >= sigt1) {

      result = trupt1(tcdot, stress, Tclad);

    } else {

      trup1 = log10( trups1(stress, Tclad) );
      trup2 = log10( trupt1(tcdot, stress, Tclad) );

      Real tmp = (stress-sigs1)/(sigt1-sigs1)*(trup2-trup1)+trup1;
      result = pow(10.0, tmp);
    }

  }
  else if (Tclad >= Tlow && Tclad <= Thigh)
  {
    Real tt1 = (2000.0 - stress)/2.6 + 273.15;
    Real sigt1 = 2000.0 - 2.6*(Tclad-273.15);

    if (stress >= sigt1) {

      result = trupt1(tcdot, stress, Tclad);

    } else {

      trup1 = log10( trups1(stress, Tclad) );
      trup2 = log10( trupt1(tcdot, stress, Tclad) );
      Real tmp = (Tclad-Tlow)/(tt1-Tlow)*(trup2-trup1)+trup1;

      result = pow(10.0, tmp);
    }

  } else if (Tclad > Thigh) {

    result = trupt1(tcdot, stress, Tclad);

  }
  return result;
}

// clad eutectic penetration rate
Real LiquidMetalCDAP::calc_deltaeut(Real temp)
{
  Real const Tlow = 1353.0;
  Real const Thigh = 1506.0;
  Real const tpeak = 1388.0;
  Real const tslow = 988.15;

  Real deltaeut = 0.0;
  if(temp <= Thigh && temp >= Tlow) {
    Real dT = temp - tpeak;
    deltaeut = 1.0e-06 * ( 922.0 + 2.9265*dT - 0.21522*dT*dT + 0.0011338*dT*dT*dT );
  } else if (temp >= tslow && temp < Tlow) {
    deltaeut = 1.0e-06*exp(22.847 - 27624.0/temp);
  } else {
    deltaeut = 0;
  }
  return deltaeut;
}

// clad sodium corrosion placeholder
Real LiquidMetalCDAP::calc_sodiumcorrosion()
{
  Real deltacor = 0.0;
  return deltacor;
}


Real LiquidMetalCDAP::cdfnormaldist(Real peak_cdf)
{
  Real x = log10(peak_cdf);
  Real z = (x-_cdfmean)/_cdfsigma;
  return 0.5*(1.0+erf(z/sqrt(2.0)));
}


Real LiquidMetalCDAP::calc_clad_stress(Real pgas, Real pcool, Real rci, Real rco)
{
  Real stress = (pgas - pcool)*(rci + rco)/2.0/(rco-rci);

  if(stress < 1.0e+5) {
    stress = 1.0e+5;
  }

  stress *= 1.0e-6;
  return stress;

 }


void LiquidMetalCDAP::calc_plenum_pressure(int i_rod, Real temp)
{
    if(_ifail[i_rod] == 1) {
    	_pgas[i_rod] = 1.0e+5;
    } else {
    	_pgas[i_rod] = _plegas0*temp/_Tplegas0;
    }
}


void
LiquidMetalCDAP::computeBreachVector(
                                     Real xchan, Real ychan,
                                     Real xrod,  Real yrod)
{
   Real rod_diameter = _rco *2.0;
  _breach_vec_x = xchan - xrod;
  _breach_vec_y = ychan - yrod;

  _breach_vec_mag = std::sqrt(_breach_vec_x*_breach_vec_x + _breach_vec_y*_breach_vec_y);
  if (_breach_vec_mag > 1e-16) {
    _breach_vec_x /= _breach_vec_mag;
    _breach_vec_y /= _breach_vec_mag;
  } else {
    _breach_vec_x = 1.0; _breach_vec_y = 0.0; _breach_vec_mag = 1.0;
  }

  _breach_loc_x = xrod + 0.5 * rod_diameter * _breach_vec_x;
  _breach_loc_y = yrod + 0.5 * rod_diameter * _breach_vec_y;
}

// 2) angle between breach direction and channel center (deg)
Real
LiquidMetalCDAP::compute_angle2D(Real xchan, Real ychan) const
{
  const Real dx = xchan - _breach_loc_x;
  const Real dy = ychan - _breach_loc_y;

  const Real dmag = std::sqrt(dx*dx + dy*dy);
  Real cos_theta = 1.0;
  if (_breach_vec_mag > 0.0 && dmag > 0.0)
    cos_theta = (_breach_vec_x*dx + _breach_vec_y*dy) / (_breach_vec_mag * dmag);

  cos_theta = std::max(Real(-1.0), std::min(Real(1.0), cos_theta));
  return std::acos(cos_theta) * 180.0 / libMesh::pi;
}

// 3) two-phase radius vs. z, θ  (cleaned OCR logic)
Real
LiquidMetalCDAP::compute2pRadius(Real zloc, Real zfail, Real theta_deg) const
{
  // base r_na (Na) from “oil” reference via property ratios
  Real r_na = _r_oil * _deqFail * std::pow(_p_ratio[_irod_fg], 0.5)
            / (_breach_diameter_ref * std::pow(_pgas_to_coolant_ratio_ref, 0.5));
  r_na *= std::pow(_mu_oil / _mu_na, 0.5)
       * std::pow (_sigma_oil / _sigma_na, 0.3);

  const Real m_na = _m_oil * std::pow(_mu_oil / _mu_na, 0.5)
                  * std::pow(_sigma_oil / _sigma_na, 0.3);

  Real two_r = 0.0;

  if (zloc < zfail && zloc > zfail - r_na)
  {
    if (theta_deg > 150.0)       two_r = _radius_frac * r_na;
    else if (theta_deg < 30.0)   two_r = r_na;
    else                         two_r = r_na - (theta_deg - 30.0) / 120.0 * (r_na - _radius_frac * r_na);
  }
  else if (zloc <= zfail - r_na)
  {
    two_r = 0.0;
  }
  else
  {
    if (theta_deg > 150.0)       two_r = _radius_frac * r_na + m_na * (zloc - zfail);
    else if (theta_deg < 30.0)   two_r = r_na                 + m_na * (zloc - zfail);
    else                         two_r = r_na - (theta_deg - 30.0) / 120.0 * (r_na - _radius_frac * r_na)
                                              + m_na * (zloc - zfail);
  }

  return std::min(two_r, _max_two_phase_radius);
}



void LiquidMetalCDAP::computeIntensifiedVoiding()
{
  // 0) Breach geometry from mesh centers (no axial index needed)
  {
    const Point ch = _tri_sch_mesh.getChannelCenter(_channel_fg);
    const Point rp = _tri_sch_mesh.getPinCenter(_irod_fg);
    computeBreachVector(ch(0), ch(1), rp(0), rp(1));
  }


  const Real mdotFG_chan_ave = _mdotFG_chan_ave;   // FG mdot apportioned per-channel basis (your cached value)

  // 1) Reference total flow area Atot at inlet row (OCR: Ax.row(1).sum())
  Real Atot_ref = 0.0;
  for (unsigned int i = 0; i < _n_channels; ++i)
  {
    auto * node_in = _subchannel_mesh.getChannelNode(i, /*iz=*/0);
    Atot_ref += (*_S_flow_soln)(node_in);
  }
  Atot_ref = std::max(Atot_ref, Real(1e-16));

  // 2) Total sodium inlet mdot (OCR uses BC * sumAx/Atot; we use measured inlet mdot)
  Real mdotNa_in_total = 0.0;
  for (unsigned int i = 0; i < _n_channels; ++i)
  {
    auto * node_in = _subchannel_mesh.getChannelNode(i, /*iz=*/0);
    mdotNa_in_total += (*_mdot_soln)(node_in);
  }

  // --- 3) Sodium density near the peak plane (local for this step) ---
  Real rho_na_for_Xtt = 0.0;
  {
    auto * node     = _subchannel_mesh.getChannelNode(_channel_fg, _peak_loc_fg);
    unsigned int znext =  _peak_loc_fg+1;
    
    if(znext > _n_cells) {
     znext = _n_cells;
    }
    
    auto * node_out = _subchannel_mesh.getChannelNode(_channel_fg, znext);
    const Real rho0 = (*_rho_soln)(node);
    const Real rho1 = (*_rho_soln)(node_out);
    rho_na_for_Xtt  = 0.5 * (rho0 + rho1);
  }

  // 4) Per-plane “intense” mask, alpha_intense, and channel HT multipliers
  for (unsigned int j = 1; j < _n_cells+1; ++j)
  {
    // clear mask row
    for (unsigned int i = 0; i < _n_channels; ++i)
      _inodeFG_Intense(j,i) = 0;

    Real sumAx_flagged = 0.0;

    // 4a) footprint mark via breach geometry (dist < two-phase radius)
    for (unsigned int i = 0; i < _n_channels; ++i)
    {
      
      const Point c = _tri_sch_mesh.getChannelCenter(i);
      const Real xchan = c(0), ychan = c(1);

      // footprint radius and angle
      const Real theta = compute_angle2D(xchan, ychan);                 // your existing helper
      const Real two_r = compute2pRadius(_zloc[j], _z_FailFG[_irod_fg], theta); // same physics as OCR (using j → zloc)

      // distance from breach OD point to channel center
      const Real dx = xchan - _breach_loc_x;
      const Real dy = ychan - _breach_loc_y;
      const Real dist = std::sqrt(dx*dx + dy*dy);

      if (dist < two_r)
      {
      	if(_inodeFG[_peak_loc_fg] == 0 && _inodeFG[j] == 0) {
      		_inodeFG_Intense(j,i) = 0;
      	} else {
        	_inodeFG_Intense(j,i) = 1;
        	auto * node = _subchannel_mesh.getChannelNode(i, j);
        	sumAx_flagged += (*_S_flow_soln)(node);
        }
      }
    }

    // 4b) alpha_intense[j] via Lockhart–Martinelli correlation (OCR recipe)
    const Real mdotNa_intense = mdotNa_in_total * (sumAx_flagged / Atot_ref);
    const Real denom          = std::max(Real(1e-16), mdotFG_chan_ave + mdotNa_intense);
    const Real x_flow_q       = mdotFG_chan_ave / denom;

    const Real mu_sod = 2.4e-4;         // Pa·s
    const Real mu_gas = 3.0e-5;         // Pa·s
    const Real rho_fg = _rhoFG[_irod_fg];

    const Real Xtt =
      std::pow((1.0 - x_flow_q) / std::max(Real(1e-16), x_flow_q), 0.9) *
      std::sqrt( std::max(Real(1e-16), rho_fg / std::max(Real(1e-16), rho_na_for_Xtt)) ) *
      std::pow( std::max(Real(1e-16), mu_sod / std::max(Real(1e-16), mu_gas)), 0.1 );

    const Real C = 20.0; // turbulent-turbulent
    const Real alpha_j = 1.0 - Xtt / std::sqrt(Xtt*Xtt + C*Xtt + 1.0);
    _alpha_intense[j] = std::clamp(alpha_j, Real(0.0), Real(1.0));

    // 4c) channel HT multipliers at plane j
    for (unsigned int i = 0; i < _n_channels; ++i)
      _HTFG_IntenseChan(j,i) = (_inodeFG_Intense(j,i) == 1) ? (1.0 - _alpha_intense[j]) : 1.0;
  }

  // 5) Aggregate to rods via channel-link fractions (MOOSE-compatible)
  // Expect: _pin_chan_links[i_pin] is vector<pair<unsigned, Real>> with (ichan, frac)
 // --- 5) Aggregate to rods/pins via channel-link fractions ---
  for (unsigned int i_pin = 0; i_pin < _n_pins; ++i_pin)
  {
    for (unsigned int j = 1; j < _n_cells+1; ++j)
    {
     
      Real sum = 0.0;   
      for (auto i_ch : _subchannel_mesh.getPinChannels(i_pin))
        {

          Real frac = 0.0;
          auto subch_type = _subchannel_mesh.getSubchannelType(i_ch);
          if (subch_type == EChannelType::CENTER || subch_type == EChannelType::CORNER)
          {
            frac = 1.0 / 6.0;
          }
          else
          {
            frac = 1.0 / 4.0;
          }

          sum += frac * _HTFG_IntenseChan(j,i_ch);
        }
      _HTFG_IntenseRod(j,i_pin) = sum;
    }
  }
}



void LiquidMetalCDAP::postfailjet(int i_rod, Real dt, Real pcool, Real Tcool, Real TgasPl, Real rho_na, Real axial_vel)
 {

// auto channel = chan_fail[rodjet[i_rod]];

 _mfg[i_rod]  = _mfg[i_rod]  - _mdote[i_rod]*dt;

 if(_forcedFail == 0) {
	_pst[i_rod] = _mfg[i_rod]/_mgas*_rgas*TgasPl/_plenum_vol;
	_rhoFG[i_rod] = _mgas*pcool/_rgas/Tcool;
 } else {
    _pst[i_rod] = _mfg[i_rod]/_mgas*_rgas*TgasPl/_plenumVol_exp;
 //   rho_na = rho_naexp;
 }


 if (_pst[i_rod] <= pcool) {
    _ijet[i_rod] = 0;
    return;
 }

 Real rhog = _mgas*_pst[i_rod]/_rgas/TgasPl;
 Real rho_cool = _mgas*pcool/_rgas/Tcool;
 Real rho0;
 Real rhoe = rhog*pow(2.0/(1.0+_gamma), 1.0/(_gamma-1.0));
// Real ucrit = pow((_rgas*TgasPl/_mgas_), 0.5);
 Real uetr = 3.24*pow((rho_na/rhoe),0.25)*pow((_sigma_na/rhoe/_deqFail),0.5);

 Real u0;
 Real d0;
 Real ue;

 _pst_crit[i_rod] = pcool*pow((1.0 + _gamma)/2.0,_gamma/(_gamma-1.0));


 if(_pst[i_rod] > _pst_crit[i_rod]) {   // sonic flow
    // isentropic
    rho0 = rhoe;
    Real pe = _pst[i_rod]*pow(2.0/(1.0+_gamma),_gamma/(_gamma-1.0));
    // isentropic
    Real Gam = _gamma*pow((2.0/(1.0+_gamma)),(_gamma+1)/(_gamma-1));
    _mdote[i_rod] = _cd*libMesh::pi/4.0*pow(_deqFail,2.0)*pow(Gam*_pst[i_rod]*rhog,0.5);
    ue = 4.0*_mdote[i_rod]/rhoe/libMesh::pi/pow(_deqFail,2);
    u0 = ue + libMesh::pi*pow(_deqFail,2.0)*(pe-pcool)/4.0/_mdote[i_rod];
    d0 = pow(2.0*(_mdote[i_rod]/libMesh::pi/u0/rho0),0.5);

 } else {  //subsonic flow
    rho0 = rho_cool;
    rhoe = rho_cool;
    // isentropic subsonic
    _mdote[i_rod] = _cd*libMesh::pi/4.0*pow(_deqFail,2.0)*pow(2.0*_gamma/(_gamma-1)*_pst[i_rod]*rhog,0.5)*
                    pow((pow(pcool/_pst[i_rod],2.0/_gamma) - pow(pcool/_pst[i_rod],(_gamma+1)/_gamma)),0.5);

    u0 = 4.0*_mdote[i_rod]/rhoe/libMesh::pi/pow(_deqFail,2);
    ue = u0;
    d0 =  _deqFail;

 }

 Real Y = 4.0*_e0*pow((rho_na/rho0),0.5)*_jetLength/d0;
 Real ui = u0/(1.0 + Y);
 Real di = d0 * pow(rho0/rho_na*(1.0+Y)*(Y+rho_na/rho0),0.5);

 Real alphai = Y/(Y + rho_na/rho0);

 Real eta = pow(27.0,0.5)/2.0*_e0*_sigma_na*pow((1.0 + rho0/rho_na),2.0)/rho0/pow(ui,2.0);
 Real f_kelvin = 0.005*(1.0 + 75.0*eta/di);
 _h_jet[i_rod] = 0.12*pow(f_kelvin,0.5)*rho_na*_cp_na*alphai*ui;  // jet heat transfer coefficient
 _temp_jet[i_rod] = Tcool;  // for fission gas only jet, it is set to coolant temperature

 Real deflection_radiant;
 if(_forcedFail == 1) {
    deflection_radiant = pow(rho_na*pow(_axial_velexp,2)/rhoe/pow(u0,2),1.3)*pow(_jetLength/_deqFail,3.0)*_deqFail/_jetLength;
 } else {
    deflection_radiant = pow(rho_na*pow(axial_vel,2)/rhoe/pow(u0,2),1.3)*pow(_jetLength/_deqFail,3.0)*_deqFail/_jetLength;
 }
 Real deflection_angle = std::atan(deflection_radiant)/libMesh::pi*180.0;

 if(deflection_angle > _crit_deflection_angle) {
    _ijet[i_rod] = 0;
    _console << "Coolant flow inertia deflected the gas jet for rod number " << _rodjet[i_rod] <<std::endl;
    _console << "deflection angle = " << deflection_angle << std::endl;

 }

// check if jet or bubbly flow forms and if there is excess pressure
 if (uetr > ue) {
    _ijet[i_rod] = 0;
        _console << "Transition from jetting to bubble formation regime for rod number " << _rodjet[i_rod]
        << " pst= " << _pst[i_rod] << " pcool= " << pcool << std::endl;
    _console <<"ue= " <<  4.0*_mdote[i_rod]/rhoe/libMesh::pi/pow(_deqFail,2) << " uetr= " << uetr << std::endl;

  _console << "deflection angle = " << deflection_angle << std::endl;
 }

 }


void LiquidMetalCDAP::executeCDAP(Real dt)
{

  if (_forcedFail == 1) {
    return;
  }

    Real tcdot = 0.0;
    _total_prob = 0.0;
    _total_survivor_prob = 0.0;

    for (unsigned int i_rod = 0; i_rod < _n_pins; ++i_rod) {

      calc_plenum_pressure(i_rod, _tcool_pin_ave(_n_cells, i_rod));

      _peak_cdf[i_rod] = 0.0;

      for (unsigned int j =1; j < _n_cells+1; ++j) {
		
		tcdot =  0.5*(_temp_pin[i_rod](j, _nrfuel+1) + _temp_pin[i_rod](j, _nrfuel+2) - 
		_temp_pin[i_rod](j, _nrfuel+1) - _temp_pin[i_rod](j, _nrfuel+2) )/dt;
		tcdot = std::max(0.0,tcdot);
		
        Real deltaeut = calc_deltaeut(_temp_pin[i_rod](j,_nrfuel));
		Real delta_sodium_corrosion = calc_sodiumcorrosion();
        _clad_wastage_in(j,i_rod) = _clad_wastage_in_old(j,i_rod) + deltaeut*dt;
        _clad_wastage_out(j,i_rod) = _clad_wastage_out_old(j,i_rod) + delta_sodium_corrosion;

		Real pcool = _pcool_pin_ave(j,i_rod);
		Real r_clad_inner = _rci + _clad_wastage_in(j,i_rod);
		Real r_clad_outer = _rco - _clad_wastage_out(j,i_rod);
        _stress(j,i_rod) = calc_clad_stress(_pgas[i_rod], pcool, r_clad_inner, r_clad_outer);

		Real tclad_ave = 0.5*(_temp_pin[i_rod](j,_nrfuel+1) + _temp_pin[i_rod](j,_nrfuel+2));
        _clad_trup(j,i_rod) = trupht9(tcdot, _stress(j,i_rod), tclad_ave);

        _clad_cdf(j,i_rod) = _clad_cdf_old(j,i_rod) + dt/_clad_trup(j,i_rod);

        if (_peak_cdf[i_rod] < _clad_cdf(j,i_rod)) {
          _peak_cdf[i_rod] = _clad_cdf(j,i_rod);
          _peak_loc[i_rod] = j;

        }
      }

    }
}



void LiquidMetalCDAP::initialize_timestepCDAP(Real dt)
{

  _time_st = _time_st + _dt;
  _clad_cdf_old = _clad_cdf;
  _clad_wastage_in_old = _clad_wastage_in;
  _clad_wastage_out_old = _clad_wastage_out;

  _total_prob = 0.0;
  _total_survivor_prob = 0.0;

  for (unsigned int i_rod = 0; i_rod < _n_pins; ++i_rod) {
    calc_plenum_pressure(i_rod, _tcool_pin_ave(_n_cells, i_rod));
  }

  if (_forcedFail == 1 && _time_st >= _forcedFailTime) {
    _ijet[_forcedPin] = 1;
    auto pcool = _pcool_exp;
    auto Tcool = _tcool_exp;
    auto TgasPl = _tgas_exp;
    auto rho_na = _rho_naexp;
	auto axial_vel = _axial_velexp;
    postfailjet(_forcedPin, dt, pcool, Tcool, TgasPl, rho_na, axial_vel);
    if(_ijet[_forcedPin] == 0) {
		_console << "Jet experiment is completed at or before time= " << _time_st << std::endl;
		std::terminate();
    }

  } else if (_forcedFail == 1 && _time_st < _forcedFailTime) {

    	_time_ijet[_forcedPin] = _time_st;
    	_ijet[_forcedPin] = _forcedPin;
    	_rodjet[_forcedPin] = _forcedPin;
    	_mfg[_forcedPin] = _pgas_exp*_plenumVol_exp*_mgas/_rgas/_tgas_exp;
    	_pst_crit[_forcedPin] = _pcool_exp*exp(0.5);
    	_mdote[_forcedPin] = 0.0;
    	_peak_loc[_forcedPin] = _n_cells;

  }


  if (_forcedFail == 1) {
 	_console << "time= " << _time_st << " ijet= " << _ijet[_forcedPin] << " heat transfer coefficient= " << _h_jet[_forcedPin] << " Temp difference= " << _target_heat_flux/_h_jet[_forcedPin] << std::endl;
    return;
  }


  for (unsigned int i_rod = 0; i_rod < _n_pins; ++i_rod) {

    if (_ijet[i_rod] == 1) {

       auto * node = _subchannel_mesh.getChannelNode(_chan_fail[_rodjet[i_rod]], _peak_loc[i_rod]);
       auto * node_out = _subchannel_mesh.getChannelNode(_chan_fail[_rodjet[i_rod]], _n_cells);
       auto rho = (*_rho_soln)(node);
       auto S = (*_S_flow_soln)(node);
       auto axial_vel = (*_mdot_soln)(node) / S / rho;
       auto pcool = std::max((*_P_soln)(node), _P_out);
       auto Tcool = (*_T_soln)(node);
       auto TgasPl = (*_T_soln)(node_out);
       auto rho_na = (*_rho_soln)(node);
       postfailjet(i_rod, dt, pcool, Tcool, TgasPl, rho_na, axial_vel);
    }


    if(_ifail[i_rod] == 0) {

        if (_peak_cdf[i_rod] < _cdfuplim) {

            _fail_prob[i_rod] = cdfnormaldist(_peak_cdf[i_rod]);

        } else {

            _fail_prob[i_rod] = 1;
            _total_fail = _total_fail + 1;
            _ifail[i_rod] = 1;

            float which_channel = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX/1.0));
            unsigned int sum_chan = 0;

		Real frac;
        for (auto i_ch : _subchannel_mesh.getPinChannels(i_rod)) {
     		auto subch_type = _subchannel_mesh.getSubchannelType(i_ch);
			if (subch_type == EChannelType::CENTER || subch_type == EChannelType::CORNER) {
				frac = 1.0/6.0;
			} else {
				frac = 1.0/4.0;
			}

		    _neig_chan[sum_chan] = i_ch;
            _chan_frac[sum_chan] = frac;
            sum_chan = sum_chan + 1;
        }

            Real cursor1 = 0;
            for (unsigned int ichan =0; ichan <sum_chan; ++ichan) {
                cursor1 = cursor1 + _chan_frac[ichan];
                if (which_channel < cursor1) {
                    _chan_fail[i_rod] = _neig_chan[ichan];

                    for (auto i_neigrod : _subchannel_mesh.getChannelPins(_chan_fail[i_rod])) {
                        if(i_neigrod != i_rod) {
                            _time_ijet[i_neigrod] = _time_st;
                            _ijet[i_neigrod] = 1;
                            _rodjet[i_neigrod] = i_rod;
                            Real temp = _tcool_pin_ave(_n_cells, i_rod);
                           	auto * node = _subchannel_mesh.getChannelNode(_chan_fail[i_rod], _peak_loc[i_rod]);
							Real pcool = std::max((*_P_soln)(node), _P_out);
                            _mfg[i_neigrod] = _pgas[i_rod]*_plenum_vol*_mgas/_rgas/temp;
                            _pst_crit[i_neigrod] = pcool*exp(0.5);
                            _mdote[i_neigrod] = 0.0;
                     		_h_jet[i_neigrod] = 1.0e+5; // initial value
                     		_temp_jet[i_neigrod] = _tcool_pin_ave(_peak_loc[i_rod], i_neigrod);
                        	_p_ratio[i_neigrod] = _pgas[i_rod]/pcool;
                        	computeFailZ(i_neigrod,_peak_loc[i_rod]);
                        }
                    }
                    //
                    break;
                }
            }
        }
    } // CDFUPLIM

    if (_ifail[i_rod] == 0) {
        _total_survivor_prob += _fail_prob[i_rod];
    }
    _total_prob += _fail_prob[i_rod];
  }

    // random number
    if (_total_prob >= _total_fail + 1) {
      float which_pin = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/_total_survivor_prob));
      Real cursor = 0.0;

      for (unsigned int i_rod = 0; i_rod < _n_pins; ++i_rod) {

        if(_ifail[i_rod] == 0) {

          cursor = cursor + _fail_prob[i_rod];
          // check for the new failure
          if(which_pin < cursor) {
            _ifail[i_rod] = 1;
            _total_fail += 1;
            float which_channel = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/1.0));
            int sum_chan = 0;
			Real frac;
        	for (auto i_ch : _subchannel_mesh.getPinChannels(i_rod)) {
     			auto subch_type = _subchannel_mesh.getSubchannelType(i_ch);
				if (subch_type == EChannelType::CENTER || subch_type == EChannelType::CORNER) {
					frac = 1.0/6.0;
				} else {
					frac = 1.0/4.0;
				}

		    	_neig_chan[sum_chan] = i_ch;
            	_chan_frac[sum_chan] = frac;
            	sum_chan = sum_chan + 1;
        	}

            Real cursor1 = 0;
            for (int ichan =0; ichan <sum_chan; ++ichan) {
              cursor1 = cursor1 + _chan_frac[ichan];
              if(which_channel < cursor1) {
                _chan_fail[i_rod] = _neig_chan[ichan];

                for (auto i_neigrod : _subchannel_mesh.getChannelPins(_chan_fail[i_rod])) {

                  if(i_neigrod != i_rod) {
                    _time_ijet[i_neigrod] = _time_st;
                    _ijet[i_neigrod] = 1;
                    _rodjet[i_neigrod] = i_rod;
                    Real temp = _tcool_pin_ave(_n_cells, i_rod);
                    auto * node = _subchannel_mesh.getChannelNode(_chan_fail[i_rod], _peak_loc[i_rod]);
					Real pcool = std::max((*_P_soln)(node), _P_out);
                    _mfg[i_neigrod] = _pgas[i_rod]*_plenum_vol*_mgas/_rgas/temp;
                    _pst_crit[i_neigrod] = pcool*exp(0.5);
                    _mdote[i_neigrod] = 0.0;
                    _h_jet[i_neigrod] = 1.0e+5;
                    _temp_jet[i_neigrod] = _tcool_pin_ave(_peak_loc[i_rod], i_neigrod);
                    _p_ratio[i_neigrod] = _pgas[i_rod]/pcool;
                    computeFailZ(i_neigrod,_peak_loc[i_rod]);
                  }
                }

                break;
              }
            }
            break;
          }

        } // ifail

      } // i_rod

  } // new failure

  _console <<  "time= " << _time_st <<  " total prob " << _total_prob << std::endl;


}


void LiquidMetalCDAP::postfail_driver(Real dt)
{


_mdotfg_tot = 0.0;
_ijet_any = 0;
int jet_count = 0;
_origin_rod.resize(_n_pins,0);

for (unsigned int i_rod = 0; i_rod < _n_pins; ++i_rod) {
	int counted = 0;
	if (_ijet[i_rod] == 1) {
		
	   auto * node = _subchannel_mesh.getChannelNode(_chan_fail[_rodjet[i_rod]], _peak_loc[i_rod]);
       auto * node_out = _subchannel_mesh.getChannelNode(_chan_fail[_rodjet[i_rod]], _n_cells);
       auto rho = (*_rho_soln)(node);
       auto S = (*_S_flow_soln)(node);
       auto axial_vel = (*_mdot_soln)(node) / S / rho;
       auto pcool = std::max((*_P_soln)(node), _P_out);
       auto Tcool = (*_T_soln)(node);
       auto TgasPl = (*_T_soln)(node_out);
       auto rho_na = (*_rho_soln)(node);
       postfailjet(i_rod, dt, pcool, Tcool, TgasPl, rho_na, axial_vel);
		
		jet_count = jet_count +1;
		_origin_rod[jet_count] = _rodjet[i_rod];
		
		for (int j =1; j < jet_count; ++j) {
			if(_rodjet[i_rod] == _origin_rod[j]) {
				counted = 1;
			}
		}	
		if(counted == 0) {
			_mdotfg_tot += _mdote[i_rod];
		}
	
		_ijet_any = 1;
		_irod_fg = i_rod;
		_channel_fg = _chan_fail[i_rod];
		_peak_loc_fg = _peak_loc[i_rod];
				
	}

}

int i_two_phase_flow = 0;

if(_ijet_any == 0) {
	for (unsigned int i_rod = 0; i_rod < _n_pins; ++i_rod) {
		if(_lengthz_FailFG[i_rod] > 0) {
			i_two_phase_flow = 1;
		}
	}
}

if(_ijet_any == 1 || i_two_phase_flow) {
	
	_timestep_counterFG0 = _timestep_counterFG;
	
	for (int k = 0; k < 2; ++k) {
		computeTrackChannelFG(dt);
		computeChannelXAlphaFric();				
		computeInletMassflowFg();

	}	
	computeIntensifiedVoiding () ;
	computeHTfg();
	
} else {
	_timestep_counterFG = 0.0;
	for (size_t i = 0; i < _HT_FG.size(); ++i) {
		_HT_FG[i] = 1.0;
	}
	_mdot_ratio = 1.0;
	_HTFG_IntenseChan.setOnes(_n_cells+1, _n_channels);
	_HTFG_IntenseRod.setOnes(_n_cells+1, _n_pins);  // columns = pins, not channels

	std::cout << "jet is deflected" <<std::endl;
}




}


void LiquidMetalCDAP::computeTrackChannelFG(Real dt)
{
	_timestep_counterFG = _timestep_counterFG0 +1;
	Real vel;
	
	if(_alpha < 1.0e-8) { // initial guess
       _alpha = 0.5;
     }
	
	
	Real Atot = 0.0;
	for (unsigned int i_ch = 0; i_ch < _subchannel_mesh.getNumOfChannels(); ++i_ch) {
	auto * node = _subchannel_mesh.getChannelNode(i_ch, 0);
    auto S = (*_S_flow_soln)(node);
    Atot += S;
	}
	_mdotFG_ej[_timestep_counterFG] = _mdotfg_tot;
	vel = _mdotfg_tot/_rhoFG[_irod_fg]/Atot/_alpha;
	_trailFG[_timestep_counterFG] = vel * dt;
	double tot_trail = 0.0;
	
	for (int i = 1; i < _timestep_counterFG + 1; ++i) {
		tot_trail += _trailFG[i];
	}
	
	if(_ijet_any == 1) {
		_lowz_boundFG = _z_FailFG[_irod_fg];
		
		if(tot_trail < _lengthz_FailFG[_irod_fg]) {
			_mdotFG_chan_ave = 0.0;
			for (int i=1; i < _timestep_counterFG + 1; ++i) {
				_mdotFG_chan_ave += _mdotFG_ej[i]*_trailFG[i];
			}
			_mdotFG_chan_ave = _mdotFG_chan_ave/tot_trail;  
			_highz_boundFG = tot_trail + _lowz_boundFG;
			auto z_cursor = _lowz_boundFG;
			
			_inodeFG[_peak_loc_fg] = 1;
			for (unsigned int j = _peak_loc_fg; j <=_n_cells; ++j) {
				z_cursor += _dz[j];
				if(z_cursor >= _lowz_boundFG && z_cursor < _highz_boundFG) {
					_inodeFG[j] = 1;
				} else {
					_inodeFG[j] = 0;
				}
			}
		// gas is fully developed above the failed axial node 
		_fully_developed_lowz = _lowz_boundFG;
		_fully_developed_highz = _highz_boundFG;
		} else {
			tot_trail = 0.0;
			for (unsigned int j = _peak_loc_fg; j <= _n_cells; ++j) {
				_inodeFG[j] = 1;
			}
		  
		    _mdotFG_chan_ave = 0.0;
		  	for (int i = _timestep_counterFG; i>0; --i) {
		  		tot_trail += _trailFG[i];
		  		if(tot_trail > _lengthz_FailFG[_irod_fg]) {
		  			_trailFG[i] = _trailFG[i] - (tot_trail - _lengthz_FailFG[_irod_fg]);
		  			_mdotFG_chan_ave += _mdotFG_ej[i]*_trailFG[i];
		  			tot_trail = _lengthz_FailFG[_irod_fg];
		  			_highz_boundFG = tot_trail + _lowz_boundFG;
		  			break;
		  		} else {
		  		
		  		_mdotFG_chan_ave += _mdotFG_ej[i] * _trailFG[i];
		  		
		  		}
		  	}		
		}
	} else {
		vel = _mdotFG_chan_ave/_rhoFG[_irod_fg]/Atot/_alpha;
		_lowz_boundFG += vel*dt;
		_highz_boundFG += vel*dt;
		_lowz_boundFG = std::min(_lowz_boundFG, _fully_developed_highz) ;
		_highz_boundFG = std::min(_highz_boundFG, _fully_developed_highz) ;
		auto z_cursor = _lowz_boundFG;
		
		_inodeFG[_peak_loc_fg] =0;
		for (unsigned int j = _peak_loc_fg; j <= _n_cells; ++j) {
			z_cursor += _dz[j];
			if(z_cursor >= _lowz_boundFG && z_cursor < _highz_boundFG) {
			_inodeFG[j] = 1;
		} else {
			_inodeFG[j] = 0;
		}
	  }
	}
	
}

void
LiquidMetalCDAP::computeChannelXAlphaFric()
{
  if (_channel_fg < 0 || _channel_fg >= _n_channels)
    return;

  // --- density average near failure location ---

  
  auto * node  = _subchannel_mesh.getChannelNode(_channel_fg, _peak_loc_fg);

  const Real rho_na = (*_rho_soln)(node);

  // --- viscosities (Pa·s) ---
  const Real mu_na  = 2.4e-4;
  const Real mu_gas = 3.0e-5;

  // --- total sodium inlet massflow (kg/s) ---
  Real mdotNa_tot = 0.0;
  for (unsigned int i = 0; i < _n_channels; ++i)
  {
    auto * node_in = _subchannel_mesh.getChannelNode(i, 0);
    mdotNa_tot += (*_mdot_soln)(node_in);
  }

  // --- average FG channel massflow (already computed elsewhere) ---
  // guard against zero
  const Real denom = std::max(_mdotFG_chan_ave + mdotNa_tot, Real(1e-16));
  const Real x_flow_q = _mdotFG_chan_ave / denom;

  // --- Lockhart–Martinelli parameter ---
  // _rhoFG[irod_fg] must be set (gas density at failure rod)
  const Real rho_fg = _rhoFG[_irod_fg];
  const Real Xtt =
      std::pow(((1.0 - x_flow_q) / std::max(x_flow_q, Real(1e-16))), 0.9) *
      std::pow(rho_fg / rho_na, 0.5) *
      std::pow(mu_na / mu_gas, 0.1);

  // --- constants ---
  const Real C = 20.0;

  // --- void fraction ---
  _alpha = 1.0 - Xtt / std::sqrt(Xtt * Xtt + C * Xtt + 1.0);

  // --- multipliers ---
  _fric_mult = 1.0 + C / std::max(Xtt, Real(1e-16)) + 1.0 / (Xtt * Xtt + 1e-16);
  _grav_mult = (1.0 - _alpha);
}


// Compute reduced two-phase sodium massflow for the failed channel.
// Uses subchannel mesh/solution to fetch per-cell flow area S(j,i).
void
LiquidMetalCDAP::computeInletMassflowFg()
{
  // total inlet sodium massflow from BC
  Real mdot_tot = 0.0;
  // --- helper to get flow area S(j,i) from subchannel mesh/solution ---
  auto getAx = [&](int j, int i_ch) -> Real {
    auto * node = _subchannel_mesh.getChannelNode(i_ch, j);
    return (*_S_flow_soln)(node);  // cross-sectional flow area for channel cell
  };

  // ---- reference single-phase massflow for the failed channel (row j=0) ----
  // sum all inlet-row areas across channels
  Real sumAx_inlet = 0.0;
  for (unsigned int i = 0; i < _n_channels; ++i) {
  	sumAx_inlet += getAx(/*j=*/0, /*i_ch=*/i);
  	auto * node = _subchannel_mesh.getChannelNode(i, 0);
  	mdot_tot += (*_mdot_soln)(node);
  	// guard against degenerate area
  }
  sumAx_inlet = std::max(sumAx_inlet, Real(1e-16));
  	// reference single-phase massflow for the failed channel
  Real mdot_ch = mdot_tot * getAx(0, _channel_fg) / sumAx_inlet;

  // ---- compute reference Δp (single-phase) along the failed channel ----
  Real dp_fric = 0.0;
  Real dp_grav = 0.0;

  for (unsigned int j = 1; j < _n_cells+1; ++j)
  {
    auto * node  = _subchannel_mesh.getChannelNode(_channel_fg, j);
    const Real Ax   = getAx(j, _channel_fg);
    const Real rho  = (*_rho_soln)(node);
    
    const Real S       = (*_S_flow_soln)(node);            // m^2
    const Real w_perim = (*_w_perim_soln)(node);           // m
    const Real Dh = 4.0 * S / w_perim;                        // m
    const Real u  = mdot_ch/ (S * std::max(rho, 1e-12));     // m/s
    const Real mu = (*_mu_soln)(node);   
    const Real Re = std::abs(u) * Dh / std::max(mu, 1e-12);
    
    _friction_args.i_ch    = _channel_fg;
    _friction_args.Re      = Re;
    _friction_args.S       = S;
    _friction_args.w_perim = w_perim;
    const Real f = computeFrictionFactor(_friction_args);
    
    const auto & k_grid = _subchannel_mesh.getKGrid();  
    
    const Real Keff = f * (_dz[j] / Dh)
                       + k_grid[_channel_fg][j-1];

    dp_fric += Keff * (mdot_ch * mdot_ch) / std::max(rho, Real(1e-16))
             / 2.0 / std::max(Ax * Ax, Real(1e-24));
    dp_grav += _GRAV * rho * _dz[j];
  }

  const Real dp_tot = dp_fric + dp_grav;

  // ---- iterative two-phase correction in FG region (same logic as OCR) ----
  Real mdot_two_phase = mdot_ch;
  Real dp_tot_tf = 0.0;

  int  k0 = -1, k1 = -1;
  Real multi = 0.05;
  int  conv  = 0;

  for (int k = 1; k <= 100; ++k)
  {
    Real dp_fric_tf = 0.0;
    Real dp_grav_tf = 0.0;

    for (unsigned int j = 1; j < _n_cells+1; ++j)
    {
     auto * node  = _subchannel_mesh.getChannelNode(_channel_fg, j);
     const Real Ax   = getAx(j, _channel_fg);
     const Real rho  = (*_rho_soln)(node);
    
     const Real S       = (*_S_flow_soln)(node);            // m^2
     const Real w_perim = (*_w_perim_soln)(node);           // m
     const Real Dh = 4.0 * S / w_perim;                        // m
     const Real u  = mdot_two_phase / (S * std::max(rho, 1e-12));     // m/s
     const Real mu = (*_mu_soln)(node);   
     const Real Re = std::abs(u) * Dh / std::max(mu, 1e-12);
    
     _friction_args.i_ch    = _channel_fg;
     _friction_args.Re      = Re;
     _friction_args.S       = S;
     _friction_args.w_perim = w_perim;
     const Real f = computeFrictionFactor(_friction_args);
    
     const auto & k_grid = _subchannel_mesh.getKGrid();  
     const Real Keff = f * (_dz[j] / Dh)
                    + k_grid[_channel_fg][j-1];
                    
     const Real base = Keff * (mdot_two_phase * mdot_two_phase)
                      / std::max(rho, Real(1e-16))
                      / 2.0 / std::max(Ax * Ax, Real(1e-24));
     const Real g    = _GRAV * rho * _dz[j];

     if (_inodeFG[j] == 1) {
     	dp_fric_tf += _fric_mult * base;
        dp_grav_tf += _grav_mult * g;
      } else {
        dp_fric_tf += base;
        dp_grav_tf += g;
      }
    }

    dp_tot_tf = dp_fric_tf + dp_grav_tf;

    if (k == 1) {
      if (dp_tot_tf > dp_tot) { k0 = -1; k1 = -1; }
      else                    { k0 =  1; k1 =  1; }
    }

    if ((k0 == -1 && k1 == 1) || (k0 == 1 && k1 == -1)) {
      multi = 0.01;
      conv  = 1;
    } else if (conv == 0) {
      multi = 0.01;
    }

    if (dp_tot_tf > dp_tot) {
      mdot_two_phase *= (1.0 - multi);
      k0 = k1; k1 = -1;
    } else {
      mdot_two_phase *= (1.0 + multi);
      k0 = k1; k1 =  1;
    }

    const Real denom = std::max(dp_tot_tf, Real(1e-16));
    const Real error = std::fabs(dp_tot_tf - dp_tot) / denom;

    if (error < 0.01) {
      _mdot_ratio = mdot_two_phase / std::max(mdot_ch, Real(1e-16));
      _mdot_ratio = std::min(1.0, _mdot_ratio);
      break;
    }
  }
}


void
LiquidMetalCDAP::computeHTfg()
{

  for (unsigned int j = 0; j < _n_cells; ++j)
  {
    if (_inodeFG[j] == 1)
    {
      // apply void fraction penalty
      _HT_FG[j] = 1.0 - _alpha;
    }
    else
    {
      // single phase
      _HT_FG[j] = 1.0;
    }
  }
}
// TODO: Check application mdot ratio as a multiplier to the inlet mass flow rate boundary condition.
void
LiquidMetalCDAP::computeMdot(int iblock)
{
  unsigned int last_node = (iblock + 1) * _block_size;
  unsigned int first_node = iblock * _block_size + 1;
  if (!_implicit_bool)
  {
    for (unsigned int iz = first_node; iz < last_node + 1; iz++)
    {
      auto dz = _z_grid[iz] - _z_grid[iz - 1];
      for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
      {
        auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
        auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
        auto volume = dz * (*_S_flow_soln)(node_in);
        auto time_term = _TR * ((*_rho_soln)(node_out)-_rho_soln->old(node_out)) * volume / _dt;
        // Wij positive out of i into j;
        auto mdot_out = (*_mdot_soln)(node_in) * _mdot_ratio - (*_SumWij_soln)(node_out)-time_term;
        if (mdot_out < 0)
        {
          _console << "Wij = : " << _Wij << "\n";
          mooseError(name(),
                     " : Calculation of negative mass flow mdot_out = : ",
                     mdot_out,
                     " Axial Level= : ",
                     iz,
                     " - Implicit solves are required for recirculating flow.");
        }
        _mdot_soln->set(node_out, mdot_out); // kg/sec
      }
    }
  }
  else
  {
    for (unsigned int iz = first_node; iz < last_node + 1; iz++)
    {
      auto dz = _z_grid[iz] - _z_grid[iz - 1];
      auto iz_ind = iz - first_node;
      for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
      {
        auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
        auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
        auto volume = dz * (*_S_flow_soln)(node_in);

        // Adding time derivative to the RHS
        auto time_term = _TR * ((*_rho_soln)(node_out)-_rho_soln->old(node_out)) * volume / _dt;
        PetscInt row_vec = i_ch + _n_channels * iz_ind;
        PetscScalar value_vec = -1.0 * time_term;
        LibmeshPetscCall(
            VecSetValues(_mc_axial_convection_rhs, 1, &row_vec, &value_vec, INSERT_VALUES));

        // Imposing bottom boundary condition or adding of diagonal elements
        if (iz == first_node)
        {
          PetscScalar value_vec = (*_mdot_soln)(node_in);
          PetscInt row_vec = i_ch + _n_channels * iz_ind;
          LibmeshPetscCall(
              VecSetValues(_mc_axial_convection_rhs, 1, &row_vec, &value_vec, ADD_VALUES));
        }
        else
        {
          PetscInt row = i_ch + _n_channels * iz_ind;
          PetscInt col = i_ch + _n_channels * (iz_ind - 1);
          PetscScalar value = -1.0;
          LibmeshPetscCall(
              MatSetValues(_mc_axial_convection_mat, 1, &row, 1, &col, &value, INSERT_VALUES));
        }

        // Adding diagonal elements
        PetscInt row = i_ch + _n_channels * iz_ind;
        PetscInt col = i_ch + _n_channels * iz_ind;
        PetscScalar value = 1.0;
        LibmeshPetscCall(
            MatSetValues(_mc_axial_convection_mat, 1, &row, 1, &col, &value, INSERT_VALUES));

        // Adding cross flows RHS
        if (_segregated_bool)
        {
          PetscScalar value_vec_2 = -1.0 * (*_SumWij_soln)(node_out);
          PetscInt row_vec_2 = i_ch + _n_channels * iz_ind;
          LibmeshPetscCall(
              VecSetValues(_mc_axial_convection_rhs, 1, &row_vec_2, &value_vec_2, ADD_VALUES));
        }
      }
    }
    LibmeshPetscCall(MatAssemblyBegin(_mc_axial_convection_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_mc_axial_convection_mat, MAT_FINAL_ASSEMBLY));

    if (_segregated_bool)
    {
      KSP ksploc;
      PC pc;
      Vec sol;
      LibmeshPetscCall(VecDuplicate(_mc_axial_convection_rhs, &sol));
      LibmeshPetscCall(KSPCreate(PETSC_COMM_WORLD, &ksploc));
      LibmeshPetscCall(KSPSetOperators(ksploc, _mc_axial_convection_mat, _mc_axial_convection_mat));
      LibmeshPetscCall(KSPGetPC(ksploc, &pc));
      LibmeshPetscCall(PCSetType(pc, PCJACOBI));
      LibmeshPetscCall(KSPSetTolerances(ksploc, _rtol, _atol, _dtol, _maxit));
      LibmeshPetscCall(KSPSetFromOptions(ksploc));
      LibmeshPetscCall(KSPSolve(ksploc, _mc_axial_convection_rhs, sol));
      LibmeshPetscCall(populateSolutionChan<SolutionHandle>(
          sol, *_mdot_soln, first_node, last_node, _n_channels));
      LibmeshPetscCall(VecZeroEntries(_mc_axial_convection_rhs));
      LibmeshPetscCall(KSPDestroy(&ksploc));
      LibmeshPetscCall(VecDestroy(&sol));
    }
  }
}



void
LiquidMetalCDAP::computeFailZ(int irod, unsigned int peakz)
{

  Real len = 0.0;

  for (unsigned int j = peakz; j < _n_cells+1; ++j)
  {
    if (j == peakz)
      len += 0.5 * _dz[j];   // half the failed cell
    else
      len += _dz[j];         // full length of cell
  }

  _lengthz_FailFG[irod] = len;

  // total length is just sum over all dz entries
  Real total_length = 0.0;
  for (unsigned int j = 1; j < _n_cells+1; ++j)
    total_length += _dz[j];

  _z_FailFG[irod] = total_length - len;
}

void
LiquidMetalCDAP::externalSolve()
{
  _console << "Executing subchannel solver\n";
  _dt = (isTransient() ? dt() : _one);
  _TR = isTransient();
  initializeSolution();
  if (_verbose_subchannel)
    _console << "Solution initialized" << std::endl;
  Real P_error = 1.0;
  unsigned int P_it = 0;
  unsigned int P_it_max;

  if (_segregated_bool)
    P_it_max = 20 * _n_blocks;
  else
    P_it_max = 100;

  if ((_n_blocks == 1) && (_segregated_bool))
    P_it_max = 5;

  while ((P_error > _P_tol && P_it < P_it_max))
  {
    P_it += 1;
    if (P_it == P_it_max && _n_blocks != 1)
    {
      _console << "Reached maximum number of axial pressure iterations" << std::endl;
      _converged = false;
    }
    _console << "Solving Outer Iteration : " << P_it << std::endl;
    auto P_L2norm_old_axial = _P_soln->L2norm();
    for (unsigned int iblock = 0; iblock < _n_blocks; iblock++)
    {
      int last_level = (iblock + 1) * _block_size;
      int first_level = iblock * _block_size + 1;
      Real T_block_error = 1.0;
      auto T_it = 0;
      _console << "Solving Block: " << iblock << " From first level: " << first_level
               << " to last level: " << last_level << std::endl;
      while (T_block_error > _T_tol && T_it < _T_maxit)
      {
        T_it += 1;
        if (T_it == _T_maxit)
        {
          _console << "Reached maximum number of temperature iterations for block: " << iblock
                   << std::endl;
          _converged = false;
        }
        auto T_L2norm_old_block = _T_soln->L2norm();

        if (_segregated_bool)
        {
          computeWijFromSolve(iblock);

          if (_compute_power)
          {
            computeh(iblock);
            computeT(iblock);
          }
        }
        else
        {
          LibmeshPetscCall(implicitPetscSolve(iblock));
          computeWijPrime(iblock);
          if (_verbose_subchannel)
            _console << "Done with main solve." << std::endl;
          if (_monolithic_thermal_bool)
          {
            // Enthalpy is already solved from the monolithic solve
            computeT(iblock);
          }
          else
          {
            if (_verbose_subchannel)
              _console << "Starting thermal solve." << std::endl;
            if (_compute_power)
            {
              computeh(iblock);
              computeT(iblock);
            }
            if (_verbose_subchannel)
              _console << "Done with thermal solve." << std::endl;
          }
        }

        if (_verbose_subchannel)
          _console << "Start updating thermophysical properties." << std::endl;

        if (_compute_density)
          computeRho(iblock);

        if (_compute_viscosity)
          computeMu(iblock);

        if (_verbose_subchannel)
          _console << "Done updating thermophysical properties." << std::endl;

        // We must do a global assembly to make sure data is parallel consistent before we do things
        // like compute L2 norms
        _aux->solution().close();

        auto T_L2norm_new = _T_soln->L2norm();
        T_block_error =
            std::abs((T_L2norm_new - T_L2norm_old_block) / (T_L2norm_old_block + 1E-14));
        _console << "T_block_error: " << T_block_error << std::endl;
      }
    }
    auto P_L2norm_new_axial = _P_soln->L2norm();
    P_error =
        std::abs((P_L2norm_new_axial - P_L2norm_old_axial) / (P_L2norm_old_axial + _P_out + 1E-14));
    _console << "P_error :" << P_error << std::endl;
    if (_verbose_subchannel)
    {
      _console << "Iteration:  " << P_it << std::endl;
      _console << "Maximum iterations: " << P_it_max << std::endl;
    }
  }
  // update old crossflow matrix
  _Wij_old = _Wij;
  _console << "Finished executing subchannel solver\n";

  if (pintemp_ss == 0)
  {

    if (pintemp_init == 0)
    {
      _console << "initializing the PinTempSolver solution" << std::endl;
      initializeSolutionPinTempSolver();
      _console << "initializing CDAP" << std::endl;
      initializeSolutionCDAP();
      pintemp_init = 1;
    }

    _console << "running PinTempSolverDriver for steady state" << std::endl;
    PinTempSolverDriver(_dt, pintemp_ss);

    if (_time > 0)
    {
      pintemp_ss = 1;
    }
  }

  // Transient PinTempSolver
  if (isTransient() > 0 && pintemp_ss == 1)
  {
    _console << "running PinTempSolverDriver for transient" << std::endl;
    PinTempSolverDriver(_dt, pintemp_ss);
    _console << "running CDAP for transient" << std::endl;
    initialize_timestepCDAP(_dt);
    executeCDAP(_dt);
 //   _console << "peak cdf " <<  _peak_cdf[0] << " stress " << _stress(_n_cells, 0) << " temp " << _temp_pin[0](_n_cells, _nrpin-1) << std::endl;
  }

  if (_pin_mesh_exist)
  {
    for (unsigned int i_pin = 0; i_pin < _n_pins; i_pin++)
    {
      for (unsigned int iz = 0; iz < _n_cells + 1; ++iz)
      {
        const auto * pin_node = _subchannel_mesh.getPinNode(i_pin, iz);
        _Tpin_soln->set(pin_node, _temp_pin[i_pin](iz, _nrpin));
      }
    }
  }
  /// Assigning temperatures to duct
  if (_duct_mesh_exist)
  {
    _console << "Commencing calculation of duct surface temperature " << std::endl;
    auto duct_nodes = _subchannel_mesh.getDuctNodes();
    for (Node * dn : duct_nodes)
    {
      auto * node_chan = _subchannel_mesh.getChannelNodeFromDuct(dn);
      auto mu = (*_mu_soln)(node_chan);
      auto S = (*_S_flow_soln)(node_chan);
      auto w_perim = (*_w_perim_soln)(node_chan);
      auto Dh_i = 4.0 * S / w_perim;
      auto Re = (((*_mdot_soln)(node_chan) / S) * Dh_i / mu);
      auto k = _fp->k_from_p_T((*_P_soln)(node_chan) + _P_out, (*_T_soln)(node_chan));
      auto cp = _fp->cp_from_p_T((*_P_soln)(node_chan) + _P_out, (*_T_soln)(node_chan));
      auto Pr = (*_mu_soln)(node_chan)*cp / k;
      /// FIXME - model assumes HTC calculation via Dittus-Boelter correlation
      auto Nu = 0.023 * std::pow(Re, 0.8) * std::pow(Pr, 0.4);
      auto hw = Nu * k / Dh_i;
      auto T_chan = (*_duct_heat_flux_soln)(dn) / hw + (*_T_soln)(node_chan);
      _Tduct_soln->set(dn, T_chan);
    }
  }
  _aux->solution().close();
  _aux->update();

  Real power_in = 0.0;
  Real power_out = 0.0;
  Real Total_surface_area = 0.0;
  Real Total_wetted_perimeter = 0.0;
  Real mass_flow_in = 0.0;
  Real mass_flow_out = 0.0;
  for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
  {
    auto * node_in = _subchannel_mesh.getChannelNode(i_ch, 0);
    auto * node_out = _subchannel_mesh.getChannelNode(i_ch, _n_cells);
    Total_surface_area += (*_S_flow_soln)(node_in);
    Total_wetted_perimeter += (*_w_perim_soln)(node_in);
    power_in += (*_mdot_soln)(node_in) * (*_h_soln)(node_in);
    power_out += (*_mdot_soln)(node_out) * (*_h_soln)(node_out);
    mass_flow_in += (*_mdot_soln)(node_in);
    mass_flow_out += (*_mdot_soln)(node_out);
  }
  auto h_bulk_out = power_out / mass_flow_out;
  auto T_bulk_out = _fp->T_from_p_h(_P_out, h_bulk_out);

  Real bulk_Dh = 4.0 * Total_surface_area / Total_wetted_perimeter;
  Real inlet_mu = (*_mu_soln)(_subchannel_mesh.getChannelNode(0, 0));
  Real bulk_Re = mass_flow_in * bulk_Dh / (inlet_mu * Total_surface_area);
  if (_verbose_subchannel)
  {
    _console << " ======================================= " << std::endl;
    _console << " ======== Subchannel Print Outs ======== " << std::endl;
    _console << " ======================================= " << std::endl;
    _console << "Total flow area :" << Total_surface_area << " m^2" << std::endl;
    _console << "Assembly hydraulic diameter :" << bulk_Dh << " m" << std::endl;
    _console << "Assembly Re number :" << bulk_Re << " [-]" << std::endl;
    _console << "Bulk coolant temperature at outlet :" << T_bulk_out << " K" << std::endl;
    _console << "Power added to coolant is : " << power_out - power_in << " Watt" << std::endl;
    _console << "Mass flow rate in is : " << mass_flow_in << " kg/sec" << std::endl;
    _console << "Mass balance is : " << mass_flow_out - mass_flow_in << " kg/sec" << std::endl;
    _console << "User defined outlet pressure is : " << _P_out << " Pa" << std::endl;
    _console << " ======================================= " << std::endl;
  }

  if (MooseUtils::absoluteFuzzyLessEqual((power_out - power_in), -1.0))
    mooseWarning(
        "Energy conservation equation might not be solved correctly, Power added to coolant:  " +
        std::to_string(power_out - power_in) + " Watt ");
}


































































