#include "SubchannelSolver.h"

#include "iapws.h"
#include "SolutionHandle.h"

//TODO: remove
#include <iostream>

registerMooseObject("MooseApp", SubchannelSolver);

template <>
InputParameters
validParams<SubchannelSolver>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addRequiredCoupledVar("vz", "axial velocity");
  params.addRequiredCoupledVar("P", "pressure");
  params.addRequiredCoupledVar("h", "specific enthalpy");
  params.addRequiredCoupledVar("T", "fluid temperature");
  params.addRequiredCoupledVar("rho", "density");
  params.addRequiredCoupledVar("flow_area", "");
  params.addRequiredCoupledVar("wetted_perimeter", "");
  params.addRequiredCoupledVar("q_prime", "linear heat rate");
  params.addParam<Real>("vz_in", 4.70, "Inlet coolant velocity in [m/s]");
  params.addParam<Real>("T_in", 566.3, "Inlet coolant temperature in [K]");
  params.addParam<Real>("P_in", 15.51, "Inlet coolant pressure in [MPa]");
  return params;
}

SubchannelSolver::SubchannelSolver(const InputParameters & params)
  : GeneralUserObject(params),
    Coupleable(this, "true"),
    _vz_var(*getFEVar("vz", 0)),
    _P_var(*getFEVar("P", 0)),
    _h_var(*getFEVar("h", 0)),
    _T_var(*getFEVar("T", 0)),
    _rho_var(*getFEVar("rho", 0)),
    _A_flow_var(*getFEVar("flow_area", 0)),
    _w_perim_var(*getFEVar("wetted_perimeter", 0)),
    _q_prime_var(*getFEVar("q_prime", 0)),
    _vz_in(getParam<Real>("vz_in")),
    _T_in(getParam<Real>("T_in")),
    _P_in(getParam<Real>("P_in"))
{
}

void
SubchannelSolver::initialize()
{
  _mesh = dynamic_cast<SubchannelMesh *>(&_fe_problem.mesh());
  if (!_mesh) {
    mooseError("Must use a SubchannelMesh");
  }
}

void
SubchannelSolver::execute()
{
  std::cout << "Executing subchannel solver\n";

  // Get handles for each variable's part of the solution vector.
  auto vz_soln = SolutionHandle(_vz_var);
  auto P_soln = SolutionHandle(_P_var);
  auto h_soln = SolutionHandle(_h_var);
  auto T_soln = SolutionHandle(_T_var);
  auto rho_soln = SolutionHandle(_rho_var);
  auto A_flow_soln = SolutionHandle(_A_flow_var);
  auto w_perim_soln = SolutionHandle(_w_perim_var);
  auto q_prime_soln = SolutionHandle(_q_prime_var);

  constexpr Real g_grav = 9.81;

  // Set the inlet conditions for each channel.
  {
    int iz = 0;
    for (int i_ch = 0; i_ch < _mesh->n_channels_; i_ch++) {
      auto * node = _mesh->nodes_[i_ch][iz];
      vz_soln.set(node, _vz_in);
      P_soln.set(node, _P_in);
      h_soln.set(node, iapws::h1(_P_in, _T_in));
      T_soln.set(node, _T_in);
      rho_soln.set(node, 1.0 / iapws::nu1(_P_in, _T_in));
    }
  }

  // Sweep upwards through the channels.
  for (int iz = 1; iz < _mesh->nz_ + 1; iz++) {
    // Compute the height of this element.
    auto dz = _mesh->z_grid_[iz] - _mesh->z_grid_[iz-1];

    for (int i_ch = 0; i_ch < _mesh->n_channels_; i_ch++) {
      // Find the nodes for the top and bottom of this element.
      auto * node_in = _mesh->nodes_[i_ch][iz-1];
      auto * node_out = _mesh->nodes_[i_ch][iz];

      // Copy the variables at the inlet (bottom) of this element.
      auto vz_in = vz_soln(node_in);
      auto P_in = P_soln(node_in);
      auto h_in = h_soln(node_in);
      auto rho_in = rho_soln(node_in);
      auto q_prime = q_prime_soln(node_in);

      // Copy/compute the geometry parameters.
      auto A_flow = A_flow_soln(node_in);
      auto w_perim = w_perim_soln(node_in);
      auto D_h = 4.0 * A_flow / w_perim;  // hydraulic diameter

      // Compute the mass and enthalpy fluxes entering through the bottom of the
      // element.
      auto mass_flux = rho_in * vz_in;
      auto enthalpy_flux = rho_in * vz_in * h_in;

      // Add the cross flow terms to the fluxes.
      for (auto i_gap : _mesh->chan_to_gap_map_[i_ch]) {
        // Get the index for the channel on the other side of this gap.
        auto ch1 = _mesh->gap_to_chan_map_[i_gap].first;
        auto ch2 = _mesh->gap_to_chan_map_[i_gap].second;
        auto j_ch = i_ch == ch1 ? ch2 : ch1;

        // Copy variables from the other channel.
        auto * node_j = _mesh->nodes_[j_ch][iz-1];
        auto vz_j = vz_soln(node_j);
        auto h_j = h_soln(node_j);
        auto rho_j = rho_soln(node_j);
        auto A_j = A_flow_soln(node_j);

        // Compute the cross flow.
        auto A_i = A_flow;
        constexpr Real gap_width = 0.0034544; //TODO: compute for each gap
        constexpr Real beta = 0.08; //TODO: parameterize
        auto avg_mass_flux = (rho_in*vz_in*A_i + rho_j*vz_j*A_j) / (A_i + A_j);
        auto w_prime = beta * gap_width * avg_mass_flux;
        enthalpy_flux += w_prime * (h_j - h_in) * dz / A_flow;
      }

      // Add the externally applied heat flux to the enthalpy.
      enthalpy_flux += q_prime * dz / A_flow;

      // Compute the outlet h from the conservation of energy.
      auto h_out = enthalpy_flux / mass_flux;
      if (h_out > iapws::h1(P_in, iapws::sat_temp(P_in))) mooseError(
        "Boiling detected.  Our water property tables are no good here!");

      // Compute the outlet P from the conservation of momentum.  Use the
      // McAdams relation for the friction coefficient.
      auto T_in = iapws::T_from_p_h(P_in, h_in);
      auto mu_in = iapws::mu(T_in, rho_in);
      auto Re_in = (rho_in * vz_in * D_h) / mu_in;
      auto fric_var = rho_in * std::pow(vz_in, 2) * 0.5;
      auto f = 0.184 * std::pow(Re_in, -0.2);
      auto fric_loss = f * dz / D_h * fric_var;
      auto grav_loss = g_grav * rho_in * dz;
      auto P_out = P_in + (-fric_loss - grav_loss) * 1e-6;

      // Compute the outlet T and rho from closure.
      auto T_out = iapws::T_from_p_h(P_out, h_out);
      auto rho_out = 1.0 / iapws::nu1(P_out, T_out);

      // Compute the outlet v_z from the conservation of mass.
      auto vz_out = mass_flux / rho_out;

      // Update the solution vectors.
      vz_soln.set(node_out, vz_out);
      P_soln.set(node_out, P_out);
      h_soln.set(node_out, h_out);
      T_soln.set(node_out, T_out);
      rho_soln.set(node_out, rho_out);
    }
  }

  std::cout << "Finished executing subchannel solver\n";
}

void
SubchannelSolver::finalize()
{
}
