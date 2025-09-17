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

#include "PinTempSolver.h"
#include "AuxiliarySystem.h"
#include "TriSubChannelMesh.h"

registerMooseObject("SubChannelApp", PinTempSolver);

InputParameters
PinTempSolver::validParams()
{
  InputParameters params = TriSubChannel1PhaseProblem::validParams();
  params.addClassDescription("Solver class for metal fuel pin temperature distribution "
                             " for liquid metal cooled metallic fuel pins");
  params.addRequiredParam<Real>("fuel_inner_radius", "fuel inner radius");
  params.addRequiredParam<Real>("fuel_outer_radius", "fuel outer radius");
  params.addRequiredParam<Real>("clad_inner_radius", "clad inner radius");
  params.addRequiredParam<Real>("gap_conductance", "gap conductance");
  params.addRequiredParam<Real>("pu_weight_fr", "Plutonium Weight Fraction");
  params.addRequiredParam<Real>("zr_weight_fr", "Zirconium Weight Fraction");
  params.addRequiredParam<Real>("por_fr", "Fuel Porosity Fraction");
  params.addRequiredParam<Real>("solidus_temp", "Fuel Solidus Temperature");
  params.addRequiredParam<Real>("liquidus_temp", "Fuel Liquidus Temperature");
  params.addRequiredParam<Real>("fusion_heat", "Fuel Heat of Fusion");
  params.addRequiredParam<unsigned int>("fuel_radial_nodes", "Number of Fuel Radial Nodes");
  return params;
}

PinTempSolver::PinTempSolver(const InputParameters & params)
  : TriSubChannel1PhaseProblem(params),
    _tri_sch_mesh(dynamic_cast<TriSubChannelMesh &>(_subchannel_mesh)),
    _r0(getParam<Real>("fuel_inner_radius")),
    _rfu(getParam<Real>("fuel_outer_radius")),
    _rci(getParam<Real>("clad_inner_radius")),
    _hgap(getParam<Real>("gap_conductance")),
    _wpu(getParam<Real>("pu_weight_fr")),
    _wzr(getParam<Real>("zr_weight_fr")),
    _por(getParam<Real>("por_fr")),
    _tsol(getParam<Real>("solidus_temp")),
    _tliq(getParam<Real>("liquidus_temp")),
    _ufmelt(getParam<Real>("fusion_heat")),
    _nrfuel(getParam<unsigned int>("fuel_radial_nodes"))
{
}

PinTempSolver::~PinTempSolver() {}

void
PinTempSolver::initializeSolutionPinTempSolver()
{
  _nrpin = _nrfuel + 2;
  _rco = 0.5 * _subchannel_mesh.getPinDiameter();

  if (_r0 < 1.0e-8)
  {
    _r0 = 1.0e-8;
  }

  if (_r0 > _rfu || _rfu > _rci || _rci > _rco)
  {
    _console << "Fuel pin Geometry Error is found. Check radial dimensions." << std::endl;
    exit(0);
  }

  _tcool_pin_ave.setZero(_n_cells + 1, _n_pins);
  _pcool_pin_ave.setZero(_n_cells + 1, _n_pins);
  _hcool_pin_ave.setZero(_n_cells + 1, _n_pins);

  _qbarconv_channel.setZero(_n_cells + 1, _n_channels);

  ijet.resize(_n_pins, 0);
  rodjet.resize(_n_pins, 0);
  peak_loc.resize(_n_pins, 0);

  h_jet.resize(_n_pins, 0);
  temp_jet.resize(_n_pins, 0);

  _temp_pin.resize(_n_pins);

  _r.resize(_nrpin + 1, 0);
  _dr.resize(_nrpin + 1, 0);

  _temp1.resize(_nrpin + 1, 0);
  _temp.resize(_nrpin + 1, 0);
  _temp0.resize(_nrpin + 1, 0);
  _qtrip.resize(_nrpin + 1, 0);

  // Outer dimension
  _a.resize(_nrpin + 1);

  // Inner rows: size and zero-fill
  for (unsigned int i = 0; i <= _nrpin; ++i)
  {
    _a[i].assign(_nrpin + 1, 0.0);
  }

  // RHS vectors
  _b.assign(_nrpin + 1, 0.0);

  _cpfuel.resize(_nrpin + 1, 0);
  _cpclad.resize(_nrpin + 1, 0);
  _kfuel.resize(_nrpin + 1, 0);
  _kclad.resize(_nrpin + 1, 0);
  _rhofuel.resize(_nrpin + 1, 0);
  _rhoclad.resize(_nrpin + 1, 0);

  _r[0] = _r0;
  _r[_nrfuel] = _rfu;

  _r[_nrfuel + 1] = _rci;
  _r[_nrfuel + 2] = _rco;

  for (unsigned int i = 1; i < _nrfuel; i++)
  {
    _r[i] = _r[0] + (_r[_nrfuel] - _r[0]) / float(_nrfuel) * i;
  }

  for (unsigned int i = 0; i < _nrpin + 1; i++)
  {

    if (i > 0 && i < _nrfuel)
    {
      _dr[i] = 0.5 * (_r[i] - _r[i - 1]) + 0.5 * (_r[i + 1] - _r[i]);
    }
    else if (i == 0)
    {
      _dr[i] = 0.5 * (_r[i + 1] - _r[i]);
    }
    else if (i == _nrfuel)
    {
      _dr[i] = 0.5 * (_r[i] - _r[i - 1]);
    }
    else
    {
      _dr[i] = 0.5 * (_r[_nrfuel + 2] - _r[_nrfuel + 1]);
    }
  }

  for (unsigned int i = 0; i < _n_pins; ++i)
  {
    _temp_pin[i].resize(_n_cells + 1, _nrpin + 1);
    auto * node_in = _subchannel_mesh.getChannelNode(i, 0);
    auto T_in = (*_T_soln)(node_in);
    for (unsigned int j = 0; j < _n_cells + 1; ++j)
    {
      for (unsigned int k = 0; k < _nrpin + 1; ++k)
      {
        _temp_pin[i](j, k) = T_in;
      }
    }
  }
}

void
PinTempSolver::set_convective_bc()
{

  auto dpin = 2.0 * _rco;
  _pcool_pin_ave.setZero();
  _tcool_pin_ave.setZero();
  _hcool_pin_ave.setZero();
  _qbarconv_channel.setZero();
  auto unheated_length_entry = _subchannel_mesh.getHeatedLengthEntry();
  auto heated_length = _subchannel_mesh.getHeatedLength();
  auto z_grid = _subchannel_mesh.getZGrid();
  Real frac = 0.0;
  for (unsigned int iz = 0; iz < _n_cells + 1; ++iz)
  {

    if (z_grid[iz] > unheated_length_entry && z_grid[iz] <= unheated_length_entry + heated_length)
    {

      for (unsigned int i_rod = 0; i_rod < _n_pins; ++i_rod)
      {

        Real sumh = 0.0;
        Real sumt = 0.0;
        Real sump = 0.0;
        for (auto i_ch : _subchannel_mesh.getPinChannels(i_rod))
        {
          auto subch_type = _subchannel_mesh.getSubchannelType(i_ch);
          if (subch_type == EChannelType::CENTER || subch_type == EChannelType::CORNER)
          {
            frac = 1.0 / 6.0;
          }
          else
          {
            frac = 1.0 / 4.0;
          }
          auto * node = _subchannel_mesh.getChannelNode(i_ch, iz);
          Real tempx = (*_T_soln)(node);
          Real pcool = (*_P_soln)(node);

          auto mu = (*_mu_soln)(node);
          auto S = (*_S_flow_soln)(node);
          auto w_perim = (*_w_perim_soln)(node);
          auto Dh_i = 4.0 * S / w_perim;
          auto Re = (((*_mdot_soln)(node) / S) * Dh_i / mu);
          auto k = _fp->k_from_p_T((*_P_soln)(node) + _P_out, (*_T_soln)(node));
          auto cp = _fp->cp_from_p_T((*_P_soln)(node) + _P_out, (*_T_soln)(node));
          auto Pr = (*_mu_soln)(node)*cp / k;
          auto Pe = Re * Pr;
          auto pitch = _subchannel_mesh.getPitch();
          auto ptod = pitch / _rco / 2.0;
          auto Nu = 4.0 + 0.33 * pow(ptod, 3.8) * pow(Pe / 100.0, 0.86) + 0.16 * pow(ptod, 5);
          auto hw = Nu * k / Dh_i;

          sump += frac * pcool;

          if (ijet[i_rod] == 1)
          {
          }
          else
          {
            sumh += frac * hw;
            sumt += frac * tempx;
          }
        }

        _pcool_pin_ave(iz, i_rod) = sump;
        if (ijet[i_rod] == 1)
        {
          _hcool_pin_ave(iz, i_rod) = h_jet[i_rod];
          _tcool_pin_ave(iz, i_rod) = temp_jet[i_rod];
        }
        else
        {
          _hcool_pin_ave(iz, i_rod) = sumh;
          _tcool_pin_ave(iz, i_rod) = sumt;
        }
      }
    }
    else
    {

      for (unsigned int i_rod = 0; i_rod < _n_pins; ++i_rod)
      {

        auto * node_in = _subchannel_mesh.getChannelNode(i_rod, 0);
        auto T_in = (*_T_soln)(node_in);

        _hcool_pin_ave(iz, i_rod) = 1.0e-15;
        _tcool_pin_ave(iz, i_rod) = T_in;
      }

      for (unsigned int i_ch = 0; i_ch < _n_channels; ++i_ch)
      {
        _qbarconv_channel(iz, i_ch) = 0.0;
      }
    }
  }

  for (unsigned int iz = 1; iz < _n_cells + 1; ++iz)
  {

    if (z_grid[iz] > unheated_length_entry && z_grid[iz] <= unheated_length_entry + heated_length)
    {

      for (unsigned int i_ch = 0; i_ch < _n_channels; ++i_ch)
      {
        for (auto i_rod : _subchannel_mesh.getChannelPins(i_ch))
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

          if (ijet[i_rod] == 1)
          {
            _qbarconv_channel(iz, i_ch) += frac * libMesh::pi * dpin * h_jet[i_rod] *
                                           (_temp_pin[i_rod](iz, _nrfuel + 2) - temp_jet[i_rod]);
          }
          else
          {
            _qbarconv_channel(iz, i_ch) +=
                frac * libMesh::pi * dpin * _hcool_pin_ave(iz, i_rod) *
                (_temp_pin[i_rod](iz, _nrfuel + 2) - _tcool_pin_ave(iz, i_rod));
          }
        }
      }
    }
  }
}

void
PinTempSolver::PinTempSolverDriver(Real dt, unsigned int pintemp_ss)
{

  // compute temperatures
  set_convective_bc();
  Real hcoef;
  Real tcool;
  auto unheated_length_entry = _subchannel_mesh.getHeatedLengthEntry();
  auto heated_length = _subchannel_mesh.getHeatedLength();
  auto z_grid = _subchannel_mesh.getZGrid();
  for (unsigned int i_pin = 0; i_pin < _n_pins; i_pin++)
  {
    for (unsigned int iz = 0; iz < _n_cells + 1; ++iz)
    {

      if (z_grid[iz] > unheated_length_entry && z_grid[iz] <= unheated_length_entry + heated_length)
      {

        hcoef = _hcool_pin_ave(iz, i_pin);
        tcool = _tcool_pin_ave(iz, i_pin);

        if (pintemp_ss < 1.e-6)
        {
          TempSolverSS(iz, i_pin, hcoef, tcool);
        }
        else
        {
          TempSolverTR(dt, iz, i_pin, hcoef, tcool);
        }
      }
    } // for i_pin
  } // for iz

  set_convective_bc();
}

Real
PinTempSolver::MetalFuelTHCON(Real temp, Real _wpu, Real _wzr, Real _por, Real ri)
{

  Real a = 17.5 * ((1.0 - 2 / 23 * _wzr) / (1.0 + 1.61 * _wzr) - 2.62 * _wpu);
  Real b = 1.54e-2 * ((1.0 + 0.061 * _wzr) / (1.0 + 1.61 * _wzr) + 0.9 * _wpu);
  Real c = 9.38e-6 * (1.0 - 2.7 * _wpu);
  Real k0 = a + b * temp + c * pow(temp, 2.0);

  Real a12 = 1.00423E+3;
  Real a13 = -0.21390;
  Real a14 = -1.1046E-5;

  Real kna = a12 + a13 * temp + a14 * pow(temp, 2);

  Real apu =
      _wpu / 0.239 /
      ((1.0 - _wpu - _wzr) / 0.238 + _wpu / 0.239 + _wzr / 0.09122); // atomic fraction of plutonium
  Real temp_alphatobeta = apu / 0.164 * (868.15 - 935.15) +
                          935.15; // alpha + delta to beta + gamma phase transition temperature
  Real pna = 0.0;
  Real pg = 0.0;

  if (temp > temp_alphatobeta || ri < 0.6 * _rfu)
  {
    pna = 0.0;
    pg = _por;
  }
  else
  { // it is conservatively assumed that only 30% of the alpha + delta phase _porosity is filled by
    // sodium
    pna = 0.3 * _por;
    pg = 0.7 * _por;
  }

  Real xp = pow((1.0 - pg), 1.5);
  Real xna = 1.0 - 3.0 * (1.0 - kna / k0) / (1.163 + 1.837 * (kna / k0)) * pna / (1.0 - pg);

  return xna * xp * k0;
}

Real
PinTempSolver::MetalFuelCP(Real temp, Real _wpu, Real _wzr)
{

  Real tempc = temp - 273.15;
  Real apu =
      _wpu / 0.239 /
      ((1.0 - _wpu - _wzr) / 0.238 + _wpu / 0.239 + _wzr / 0.09122); // atomic fraction of plutonium
  Real temp_alphatobeta = apu / 0.164 * (868.15 - 935.15) + 935.15 -
                          273.15; // alpha + delta to beta + gamma phase transition temperature
  Real temp_betatogamma = apu / 0.164 * (923.15 - 965.15) + 965.15 -
                          273.15; // beta + gamma to single gamma phase transition temperature
  Real mavg =
      pow(((1.0 - _wpu - _wzr) / 238.0 + _wpu / 239.0 + _wzr / 91.22), -1.0); // average molar mass
  Real cp;

  if (tempc < temp_alphatobeta)
  {
    cp = 26.58 + 0.027 / mavg * tempc;
  }
  else if (tempc > temp_betatogamma)
  {
    cp = 15.84 + 0.026 / mavg * tempc;
  }
  else
  {
    auto cp1 = 26.58 + 0.027 / mavg * temp_alphatobeta;
    auto cp2 = 15.84 + 0.026 / mavg * temp_betatogamma;
    cp = (cp2 - cp1) / (temp_betatogamma - temp_alphatobeta) * (tempc - temp_alphatobeta) + cp1;
  }

  return cp;
}

Real
PinTempSolver::MetalFuelRHO(Real _wpu, Real _wzr, Real _por)
{

  Real rho_u = 19000.0;
  Real rho_pu = 19000.0;
  Real rho_zr = 6490.0;

  Real rho_mix = (1.0 - _por) * 0.988 *
                 pow((_wpu / rho_pu + _wzr / rho_zr + (1.0 - _wpu - _wzr) / rho_u), -1.0);
  // thermal expansion has not been added yet...

  return rho_mix;
}

Real
PinTempSolver::HT9CladTHCON(Real temp)
{

  if (temp < 1030.0)
  {
    return 17.622 + 2.42e-2 * temp - 1.696e-5 * pow(temp, 2);
  }
  else
  {
    return 12.027 + 1.218e-2 * temp;
  }
}

Real
PinTempSolver::HT9CladCP(Real temp)
{

  Real tempc = temp - 273.15;

  if (tempc < 527.0)
  {
    return 1.0 / 6.0 * (tempc - 227.0) + 500.0;
  }
  else
  {
    return 3.0 / 5.0 * (tempc - 527.0) + 550.0;
  }
}

Real
PinTempSolver::HT9CladRHO()
{

  Real rho = 7800.0;
  // thermal expansion has not been added yet...

  return rho;
}

void
PinTempSolver::TempSolverSS(unsigned int iz, unsigned int i_pin, Real hcoef, Real tcool)
{

  // zero matrices/vectors for this (iz, i_pin) assembly
  for (unsigned int i = 0; i < _nrpin + 1; ++i)
  {
    std::fill(_a[i].begin(), _a[i].end(), 0.0);
    std::fill(_b.begin(), _b.end(), 0.0);
  }

  auto * pin_node = _subchannel_mesh.getPinNode(i_pin, iz);

  for (unsigned int i = 0; i < _nrpin + 1; i++)
  {
    _cpfuel[i] = MetalFuelCP(_temp_pin[i_pin](iz, i), _wpu, _wzr);
    _cpclad[i] = HT9CladCP(_temp_pin[i_pin](iz, i));
    _kfuel[i] = MetalFuelTHCON(_temp_pin[i_pin](iz, i), _wpu, _wzr, _por, _r[i]);
    _kclad[i] = HT9CladTHCON(_temp_pin[i_pin](iz, i));
    _rhofuel[i] = MetalFuelRHO(_wpu, _wzr, _por);
    _rhoclad[i] = HT9CladRHO();
    _qtrip[i] = (*_q_prime_soln)(pin_node) / libMesh::pi / (pow(_rfu, 2) - pow(_r0, 2));
    _b[i] = 0.0;
  }

  for (unsigned int i = 0; i < _nrpin + 1; i++)
  {

    if (i > 0 && i < _nrfuel)
    {

      _a[i][i] = 0.0;

      _a[i][i] = _a[i][i] + (_kfuel[i] * _dr[i] / 2.0 + _kfuel[i + 1] * _dr[i + 1] / 2.0) /
                                (_dr[i] / 2.0 + _dr[i + 1] / 2.0) * (_r[i] + _r[i + 1]) / 2.0 /
                                (_r[i + 1] - _r[i]);
      _a[i][i + 1] = -(_kfuel[i] * _dr[i] / 2.0 + _kfuel[i + 1] * _dr[i + 1] / 2.0) /
                     (_dr[i] / 2.0 + _dr[i + 1] / 2.0) * (_r[i] + _r[i + 1]) / 2.0 /
                     (_r[i + 1] - _r[i]);

      _a[i][i] = _a[i][i] + (_kfuel[i] * _dr[i] / 2.0 + _kfuel[i - 1] * _dr[i - 1] / 2.0) /
                                (_dr[i] / 2.0 + _dr[i - 1] / 2.0) * (_r[i] + _r[i - 1]) / 2.0 /
                                (_r[i] - _r[i - 1]);
      _a[i][i - 1] = -(_kfuel[i] * _dr[i] / 2.0 + _kfuel[i - 1] * _dr[i - 1] / 2.0) /
                     (_dr[i] / 2.0 + _dr[i - 1] / 2.0) * (_r[i] + _r[i - 1]) / 2.0 /
                     (_r[i] - _r[i - 1]);

      _b[i] = _qtrip[i] * _dr[i] * _r[i];
    }
    else if (i == 0)
    {

      _a[i][i] = 0.0;

      _a[i][i] = _a[i][i] + (_kfuel[i] * _dr[i] + _kfuel[i + 1] * _dr[i + 1] / 2.0) /
                                (_dr[i] + _dr[i + 1] / 2.0) * (_r[i] + _r[i + 1]) / 2.0 /
                                (_r[i + 1] - _r[i]);
      _a[i][i + 1] = -(_kfuel[i] * _dr[i] + _kfuel[i + 1] * _dr[i + 1] / 2.0) /
                     (_dr[i] + _dr[i + 1] / 2.0) * (_r[i] + _r[i + 1]) / 2.0 / (_r[i + 1] - _r[i]);
      _b[i] = _qtrip[i] * _dr[i] * (_r[i] + _dr[i] / 2.0);
    }
    else if (i == _nrfuel)
    {

      _a[i][i] = 0.0;

      _a[i][i] = _a[i][i] + (_kfuel[i] * _dr[i] + _kfuel[i - 1] * _dr[i - 1] / 2.0) /
                                (_dr[i] + _dr[i - 1] / 2.0) * (_r[i] + _r[i - 1]) / 2.0 /
                                (_r[i] - _r[i - 1]);
      _a[i][i - 1] = -(_kfuel[i] * _dr[i] + _kfuel[i - 1] * _dr[i - 1] / 2.0) /
                     (_dr[i] + _dr[i - 1] / 2.0) * (_r[i] + _r[i - 1]) / 2.0 / (_r[i] - _r[i - 1]);
      _a[i][i] = _a[i][i] + _hgap * (_r[i] + _r[i + 1]) / 2.0;
      _a[i][i + 1] = -_hgap * (_r[i] + _r[i + 1]) / 2.0;

      _b[i] = _qtrip[i] * _dr[i] * (_r[i] - _dr[i] / 2.0);
    }
    else if (i == _nrfuel + 1)
    {

      _a[i][i] = 0.0;
      _a[i][i] = _a[i][i] + _hgap * (_r[i] + _r[i - 1]) / 2.0;
      _a[i][i - 1] = -_hgap * (_r[i] + _r[i - 1]) / 2.0;
      _a[i][i] = _a[i][i] + (_kclad[i] * _dr[i] + _kclad[i + 1] * _dr[i + 1]) /
                                (_dr[i] + _dr[i + 1]) * (_r[i] + _r[i + 1]) / 2.0 /
                                (_r[i + 1] - _r[i]);
      _a[i][i + 1] = -(_kclad[i] * _dr[i] + _kclad[i + 1] * _dr[i + 1]) / (_dr[i] + _dr[i + 1]) *
                     (_r[i] + _r[i + 1]) / 2.0 / (_r[i + 1] - _r[i]);
      _b[i] = 0.0;
    }
    else if (i == _nrfuel + 2)
    {

      _a[i][i] = 0.0;
      _a[i][i] = _a[i][i] + (_kclad[i] * _dr[i] + _kclad[i - 1] * _dr[i - 1]) /
                                (_dr[i] + _dr[i - 1]) * (_r[i] + _r[i - 1]) / 2.0 /
                                (_r[i] - _r[i - 1]);
      _a[i][i - 1] = -(_kclad[i] * _dr[i] + _kclad[i - 1] * _dr[i - 1]) / (_dr[i] + _dr[i - 1]) *
                     (_r[i] + _r[i - 1]) / 2.0 / (_r[i] - _r[i - 1]);
      _a[i][i] = _a[i][i] + hcoef * _r[i];
      _b[i] = hcoef * _r[i] * tcool;
    }
  } // if _nrpin

  // solver
  Real sum1;
  Real rel_error = 0.0;
  int converged = 0;
  for (int k = 1; k < 1000; k++)
  {
    _temp1 = _temp;

    for (unsigned int i = 0; i < _nrpin + 1; i++)
    {

      if (i == 0)
      {
        sum1 = _a[i][i + 1] * _temp1[i + 1];
      }
      else if (i == _nrpin)
      {
        sum1 = _a[i][i - 1] * _temp1[i - 1];
      }
      else
      {
        sum1 = _a[i][i + 1] * _temp1[i + 1] + _a[i][i - 1] * _temp1[i - 1];
      }
      _temp[i] = (_b[i] - sum1) / _a[i][i];

    } // i

    Real rel_error_num = 0.0;
    Real rel_error_denom = 0.0;
    for (unsigned int i = 0; i < _nrpin + 1; i++)
    {
      rel_error_num = rel_error_num + abs(pow(_temp[i], 2) - pow(_temp1[i], 2));
      rel_error_denom = rel_error_denom + pow(_temp[i], 2);
    }
    rel_error = rel_error_num / rel_error_denom;
    if (rel_error < 1.0e-5)
    {
      converged = 1;
      break;
    }
  } // k

  if (converged == 0)
  {
    _console << "Error: Tsolv did not converge at tempsolvss. " << "error " << rel_error
             << " hcoef " << hcoef << " tcool " << tcool << std::endl;
    for (unsigned int i = 0; i < _nrpin + 1; i++)
    {
      _console << " node " << i << " _qtrip " << _qtrip[i] << " temp " << _temp[i] << " b " << _b[i]
               << " a " << _a[i][i] << std::endl;
    }
    exit(0);
  }

  for (unsigned int i = 0; i < _nrpin + 1; i++)
  {
    _temp_pin[i_pin](iz, i) = _temp[i];
  }
}

void
PinTempSolver::TempSolverTR(Real dt, unsigned int iz, unsigned int i_pin, Real hcoef, Real tcool)
{

  auto * pin_node = _subchannel_mesh.getPinNode(i_pin, iz);

  for (unsigned int i = 0; i <= _nrpin; ++i)
  {
    std::fill(_a[i].begin(), _a[i].end(), 0.0);
  }
  std::fill(_b.begin(), _b.end(), 0.0);

  for (unsigned int i = 0; i < _nrpin + 1; i++)
  {
    _cpfuel[i] = MetalFuelCP(_temp_pin[i_pin](iz, i), _wpu, _wzr);
    _cpclad[i] = HT9CladCP(_temp_pin[i_pin](iz, i));
    _kfuel[i] = MetalFuelTHCON(_temp_pin[i_pin](iz, i), _wpu, _wzr, _por, _r[i]);
    _kclad[i] = HT9CladTHCON(_temp_pin[i_pin](iz, i));
    _rhofuel[i] = MetalFuelRHO(_wpu, _wzr, _por);
    _rhoclad[i] = HT9CladRHO();
    _qtrip[i] = (*_q_prime_soln)(pin_node) / libMesh::pi / (pow(_rfu, 2) - pow(_r0, 2));
    _b[i] = 0.0;
  }

  // set the matrix and vector...
  for (unsigned int i = 0; i < _nrpin + 1; i++)
  {
    _temp[i] = _temp_pin[i_pin](iz, i);
  }

  _temp0 = _temp;

  for (unsigned int i = 0; i < _nrpin + 1; i++)
  {

    if (i > 0 && i < _nrfuel)
    {

      _a[i][i] = _rhofuel[i] * _cpfuel[i] * _r[i] * _dr[i] / dt;

      _a[i][i] = _a[i][i] + (_kfuel[i] * _dr[i] / 2.0 + _kfuel[i + 1] * _dr[i + 1] / 2.0) /
                                (_dr[i] / 2.0 + _dr[i + 1] / 2.0) * (_r[i] + _r[i + 1]) / 2.0 /
                                (_r[i + 1] - _r[i]);
      _a[i][i + 1] = -(_kfuel[i] * _dr[i] / 2.0 + _kfuel[i + 1] * _dr[i + 1] / 2.0) /
                     (_dr[i] / 2.0 + _dr[i + 1] / 2.0) * (_r[i] + _r[i + 1]) / 2.0 /
                     (_r[i + 1] - _r[i]);

      _a[i][i] = _a[i][i] + (_kfuel[i] * _dr[i] / 2.0 + _kfuel[i - 1] * _dr[i - 1] / 2.0) /
                                (_dr[i] / 2.0 + _dr[i - 1] / 2.0) * (_r[i] + _r[i - 1]) / 2.0 /
                                (_r[i] - _r[i - 1]);
      _a[i][i - 1] = -(_kfuel[i] * _dr[i] / 2.0 + _kfuel[i - 1] * _dr[i - 1] / 2.0) /
                     (_dr[i] / 2.0 + _dr[i - 1] / 2.0) * (_r[i] + _r[i - 1]) / 2.0 /
                     (_r[i] - _r[i - 1]);

      _b[i] =
          _temp0[i] * _rhofuel[i] * _cpfuel[i] * _r[i] * _dr[i] / dt + _qtrip[i] * _dr[i] * _r[i];
    }
    else if (i == 0)
    {

      _a[i][i] = _rhofuel[i] * _cpfuel[i] * (_r[i] + _dr[i] / 2.0) * _dr[i] / dt;

      _a[i][i] = _a[i][i] + (_kfuel[i] * _dr[i] + _kfuel[i + 1] * _dr[i + 1] / 2.0) /
                                (_dr[i] + _dr[i + 1] / 2.0) * (_r[i] + _r[i + 1]) / 2.0 /
                                (_r[i + 1] - _r[i]);
      _a[i][i + 1] = -(_kfuel[i] * _dr[i] + _kfuel[i + 1] * _dr[i + 1] / 2.0) /
                     (_dr[i] + _dr[i + 1] / 2.0) * (_r[i] + _r[i + 1]) / 2.0 / (_r[i + 1] - _r[i]);
      _b[i] = _temp0[i] * _rhofuel[i] * _cpfuel[i] * (_r[i] + _dr[i] / 2.0) * _dr[i] / dt +
              _qtrip[i] * _dr[i] * (_r[i] + _dr[i] / 2.0);
    }
    else if (i == _nrfuel)
    {

      _a[i][i] = _rhofuel[i] * _cpfuel[i] * (_r[i] - _dr[i] / 2.0) * _dr[i] / dt;

      _a[i][i] = _a[i][i] + (_kfuel[i] * _dr[i] + _kfuel[i - 1] * _dr[i - 1] / 2.0) /
                                (_dr[i] + _dr[i - 1] / 2.0) * (_r[i] + _r[i - 1]) / 2.0 /
                                (_r[i] - _r[i - 1]);
      _a[i][i - 1] = -(_kfuel[i] * _dr[i] + _kfuel[i - 1] * _dr[i - 1] / 2.0) /
                     (_dr[i] + _dr[i - 1] / 2.0) * (_r[i] + _r[i - 1]) / 2.0 / (_r[i] - _r[i - 1]);
      _a[i][i] = _a[i][i] + _hgap * (_r[i] + _r[i + 1]) / 2.0;
      _a[i][i + 1] = -_hgap * (_r[i] + _r[i + 1]) / 2.0;

      _b[i] = _temp0[i] * _rhofuel[i] * _cpfuel[i] * (_r[i] - _dr[i] / 2.0) * _dr[i] / dt +
              _qtrip[i] * _dr[i] * (_r[i] - _dr[i] / 2.0);
    }
    else if (i == _nrfuel + 1)
    {

      _a[i][i] = _rhoclad[i] * _cpclad[i] * (_r[i] + _dr[i] / 2.0) * _dr[i] / dt;
      _a[i][i] = _a[i][i] + _hgap * (_r[i] + _r[i - 1]) / 2.0;
      _a[i][i - 1] = -_hgap * (_r[i] + _r[i - 1]) / 2.0;
      _a[i][i] = _a[i][i] + (_kclad[i] * _dr[i] + _kclad[i + 1] * _dr[i + 1]) /
                                (_dr[i] + _dr[i + 1]) * (_r[i] + _r[i + 1]) / 2.0 /
                                (_r[i + 1] - _r[i]);
      _a[i][i + 1] = -(_kclad[i] * _dr[i] + _kclad[i + 1] * _dr[i + 1]) / (_dr[i] + _dr[i + 1]) *
                     (_r[i] + _r[i + 1]) / 2.0 / (_r[i + 1] - _r[i]);
      _b[i] = _temp0[i] * _rhoclad[i] * _cpclad[i] * (_r[i] + _dr[i] / 2.0) * _dr[i] / dt;
    }
    else if (i == _nrfuel + 2)
    {

      _a[i][i] = _rhoclad[i] * _cpclad[i] * (_r[i] - _dr[i] / 2.0) * _dr[i] / dt;
      _a[i][i] = _a[i][i] + (_kclad[i] * _dr[i] + _kclad[i - 1] * _dr[i - 1]) /
                                (_dr[i] + _dr[i - 1]) * (_r[i] + _r[i - 1]) / 2.0 /
                                (_r[i] - _r[i - 1]);
      _a[i][i - 1] = -(_kclad[i] * _dr[i] + _kclad[i - 1] * _dr[i - 1]) / (_dr[i] + _dr[i - 1]) *
                     (_r[i] + _r[i - 1]) / 2.0 / (_r[i] - _r[i - 1]);
      _a[i][i] = _a[i][i] + hcoef * _r[i];
      _b[i] = _temp0[i] * _rhoclad[i] * _cpclad[i] * (_r[i] - _dr[i] / 2.0) * _dr[i] / dt +
              hcoef * _r[i] * tcool;
    }
  } // if _nrpin

  // solver
  Real sum1;
  int converged = 0;
  for (int k = 1; k < 1000; k++)
  {
    _temp1 = _temp;
    for (unsigned int i = 0; i < _nrpin + 1; i++)
    {
      if (i == 0)
      {
        sum1 = _a[i][i + 1] * _temp1[i + 1];
      }
      else if (i == _nrpin)
      {
        sum1 = _a[i][i - 1] * _temp1[i - 1];
      }
      else
      {
        sum1 = _a[i][i + 1] * _temp1[i + 1] + _a[i][i - 1] * _temp1[i - 1];
      }
      _temp[i] = (_b[i] - sum1) / _a[i][i];
    } // i

    Real rel_error = 0.0;
    Real rel_error_num = 0.0;
    Real rel_error_denom = 0.0;
    for (unsigned int i = 0; i < _nrpin + 1; i++)
    {
      rel_error_num = rel_error_num + abs(pow(_temp[i], 2) - pow(_temp1[i], 2));
      rel_error_denom = rel_error_denom + pow(_temp[i], 2);
    }
    rel_error = rel_error_num / rel_error_denom;
    if (rel_error < 1.0e-5)
    {
      converged = 1;
      break;
    }
  } // k

  if (converged == 0)
  {
    _console << "Error: tsol did not converge at tempsolvtr." << std::endl;
    exit(0);
  }

  for (unsigned int i = 0; i < _nrpin + 1; i++)
  {
    _temp_pin[i_pin](iz, i) = _temp[i];
  }
}

Real
PinTempSolver::computeAddedHeatPin(unsigned int i_ch, unsigned int iz)
{
  auto dz = _z_grid[iz] - _z_grid[iz - 1];

  auto subch_type = _subchannel_mesh.getSubchannelType(i_ch);
  // If pin mesh exists, then project pin power to subchannel

  if (_time <= 0.0)
  {
    if (_pin_mesh_exist)
    {
      auto heat_rate_in = 0.0;
      auto heat_rate_out = 0.0;
      if (subch_type == EChannelType::CENTER)
      {
        for (unsigned int j = 0; j < 3; j++)
        {
          auto i_pin = _subchannel_mesh.getChannelPins(i_ch)[j];
          auto * node_in = _subchannel_mesh.getPinNode(i_pin, iz - 1);
          auto * node_out = _subchannel_mesh.getPinNode(i_pin, iz);
          heat_rate_out += (*_q_prime_soln)(node_out);
          heat_rate_in += (*_q_prime_soln)(node_in);
        }

        return 1.0 / 6.0 * (heat_rate_in + heat_rate_out) * dz / 2.0;
      }
      else if (subch_type == EChannelType::EDGE)
      {
        for (unsigned int j = 0; j < 2; j++)
        {
          auto i_pin = _subchannel_mesh.getChannelPins(i_ch)[j];
          auto * node_in = _subchannel_mesh.getPinNode(i_pin, iz - 1);
          auto * node_out = _subchannel_mesh.getPinNode(i_pin, iz);
          heat_rate_out += (*_q_prime_soln)(node_out);
          heat_rate_in += (*_q_prime_soln)(node_in);
        }

        return 1.0 / 4.0 * (heat_rate_in + heat_rate_out) * dz / 2.0;
      }
      else
      {
        auto i_pin = _subchannel_mesh.getChannelPins(i_ch)[0];
        auto * node_in = _subchannel_mesh.getPinNode(i_pin, iz - 1);
        auto * node_out = _subchannel_mesh.getPinNode(i_pin, iz);
        heat_rate_out += (*_q_prime_soln)(node_out);
        heat_rate_in += (*_q_prime_soln)(node_in);

        return 1.0 / 6.0 * (heat_rate_in + heat_rate_out) * dz / 2.0;
      }
    }
    else
    {
      auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
      auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
      return ((*_q_prime_soln)(node_out) + (*_q_prime_soln)(node_in)) * dz / 2.0;
    }
  }
  else
  {

    return (_qbarconv_channel(iz, i_ch) + _qbarconv_channel(iz - 1, i_ch)) * dz / 2.0;
  }
}

void
PinTempSolver::externalSolve()
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
      _console << "initializing te PinTempSolver solution" << std::endl;
      initializeSolutionPinTempSolver();
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
