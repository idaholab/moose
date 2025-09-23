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

#pragma once

#include "TriSubChannel1PhaseProblem.h"
#include <memory>
#include <Eigen/Dense>

/**
 * @class PinTempSolver
 * @brief 1-D, axisymmetric radial heat-conduction solver for SFR pins coupled to the
 *        Subchannel module flow/energy solver.
 *
 * This object computes the radial temperature distribution for each pin at every axial
 * location and returns the *convective* wall heat flux to the subchannel energy equation.
 * Conversely, it consumes the outer convective boundary condition (HTC and bulk coolant
 * temperature) computed by the subchannel solver.
 *
 * Key implementation notes:
 *  - Inherits from TriSubChannel1PhaseProblem to run inside the existing subchannel
 *    outer iteration.
 *  - Overrides externalSolve() to execute the pin conduction solve each time step.
 *  - Overrides computeAddedHeatPin() to deposit *convective* wall heat (for t>0) rather
 *    than raw pin power used at initialization (t<=0).
 *
 * Units: SI (m, kg, s, K); heat/energy in W, J; HTC in W/m^2/K; pressure in Pa.
 */
class PinTempSolver : public TriSubChannel1PhaseProblem
{
public:
  /// Construct with MOOSE input parameters defined in validParams()
  PinTempSolver(const InputParameters & params);

  /// Virtual destructor
  virtual ~PinTempSolver() override;

  /**
   * @brief One-time initialization for the pin-temperature solver data structures.
   *
   * Builds the radial mesh (fuel/gap/clad), allocates vectors/matrices, and seeds initial
   * temperatures (typically with inlet/bulk values).
   */
  virtual void initializeSolutionPinTempSolver();

  /** @name Jet override data (optional, per-pin)
   *  Pins flagged as "jet" use these values instead of the averaged subchannel HTC/temperature.
   *  @{
   */
  /// Flag indicating whether a jet is active on a pin (0/1)
  std::vector<unsigned int> _ijet;
  /// Rod index for the originating jet (pin id)
  std::vector<unsigned int> _rodjet;
  /// Axial index of peak CDF location for the associated failure/jet model
  std::vector<unsigned int> _peak_loc;
  /// Jet heat transfer coefficient [W/m^2/K]
  std::vector<Real> _h_jet;
  /// Jet reference temperature [K]
  std::vector<Real> _temp_jet;
  int _ijet_any;
 
  /** @name Geometry & discretization (radii in meters) @{ */

  /// Cladding outer radius r_co [m] (≈ D/2 from mesh)
  Real _rco;  
  

  unsigned int _nrpin;   ///< Total pin radial nodes (fuel + gap/clad)
  /** @} */

  Real r0()   const { return _r0; }
  Real rfu()  const { return _rfu; }
  Real rci()  const { return _rci; }
  unsigned nrfuel() const { return _nrfuel; }

  /**
   * @brief Define and document MOOSE input parameters for PinTempSolver.
   *
   * Expected parameters include geometry (fuel/clad radii), gap conductance, fuel composition
   * (Pu, Zr weight fractions), porosity, optional solidus/liquidus/fusion-heat placeholders,
   * and the number of fuel radial nodes.
   */
  static InputParameters validParams();

protected:
  /// Access to the subchannel triangular mesh (pins/channels/geometry)
  TriSubChannelMesh & _tri_sch_mesh;

  /**
   * @brief Overridden outer solve hook executed each time step.
   *
   * Calls the subchannel flow/thermal updates, initializes the pin solver,
   * performs steady/transient radial solves per (pin, axial cell), updates wall heat
   * fluxes for the subchannel energy equation, and publishes auxiliary fields.
   */
  virtual void externalSolve() override;

  /**
   * @brief Driver at the beginning of a time step.
   * @param dt         Time step size [s]
   * @param pintemp_ss Steady-state flag (0 = steady, 1 = transient)
   *
   * Executes the per-pin axial loops, assembles boundary conditions, and solves either
   * steady-state or transient radial systems.
   */
  virtual void PinTempSolverDriver(Real dt, unsigned int pintemp_ss);

  /**
   * @brief Transient radial temperature solve for a single (pin, axial) location.
   * @param dt     Time step size [s]
   * @param iz     Axial index
   * @param i_pin  Pin index
   * @param hcoef  Outer-wall HTC [W/m^2/K]
   * @param tcool  Coolant reference temperature [K]
   */
  void TempSolverTR(Real dt, unsigned int iz, unsigned int i_pin, Real hcoef, Real tcool);

  /**
   * @brief Steady-state radial temperature solve for a single (pin, axial) location.
   * @param iz     Axial index
   * @param i_pin  Pin index
   * @param hcoef  Outer-wall HTC [W/m^2/K]
   * @param tcool  Coolant reference temperature [K]
   */
  void TempSolverSS(unsigned int iz, unsigned int i_pin, Real hcoef, Real tcool);

  /**
   * @brief Build/update the outer convective boundary conditions per pin and axial cell.
   *
   * Computes area-weighted averages of channel properties surrounding each pin and forms
   * the wall HTC/temperature. For pins with active jets, uses jet overrides.
   */
  void set_convective_bc();

  /**
   * @brief Energy source returned to the subchannel energy equation.
   * @param i_ch Channel index
   * @param iz   Axial index
   * @return Heat added to channel (convective from adjacent pins) [W]
   *
   * Behavior:
   *  - t <= 0: uses pin linear power projection (initialization).
   *  - t > 0 : uses convective wall heat \f$ \sum \pi D h (T_w - T_\infty) \f$.
   */
  Real computeAddedHeatPin(unsigned int i_ch, unsigned int iz) override;

  /** @name Material property models (metallic fuel U–Pu–Zr, HT9 cladding)
   *  Temperatures are in Kelvin; densities in kg/m^3; cp in J/kg/K; k in W/m/K.
   *  @{
   */
  /// Metallic fuel thermal conductivity k_f(T; w_Pu, w_Zr, porosity) [W/m/K]
  Real MetalFuelTHCON(Real temp, Real _wpu, Real _wzr, Real _por, Real ri);
  /// Metallic fuel specific heat cp_f(T; w_Pu, w_Zr) [J/kg/K]
  Real MetalFuelCP(Real temp, Real _wpu, Real _wzr);
  /// Metallic fuel density rho_f(w_Pu, w_Zr, porosity) [kg/m^3]
  Real MetalFuelRHO(Real _wpu, Real _wzr, Real _por);
  /// HT9 cladding thermal conductivity k_c(T) [W/m/K]
  Real HT9CladTHCON(Real temp);
  /// HT9 cladding specific heat cp_c(T) [J/kg/K]
  Real HT9CladCP(Real temp);
  /// HT9 cladding density rho_c [kg/m^3]
  Real HT9CladRHO();
  /** @} */

  /** @name Solver mode and initialization flags @{ */
  /// Initialization guard (0 = not initialized, 1 = initialized)
  unsigned int pintemp_init = 0;
  /// Steady-state flag (0 = steady solve, 1 = transient solve)
  unsigned int pintemp_ss = 0;
  /** @} */

  Real _r0;              ///< Fuel inner radius r0 [m] (use ~0 for solid fuel)
  Real _rfu;             ///< Fuel outer radius r_fu [m]
  Real _rci;             ///< Cladding inner radius r_ci [m]
  Real _hgap;            ///< Effective fuel–clad gap conductance h_gap [W/m^2/K]
  Real _wpu;             ///< Pu weight fraction [-]
  Real _wzr;             ///< Zr weight fraction [-]
  Real _por;             ///< Fuel porosity fraction [-]
  Real _tsol;            ///< Fuel solidus temperature [K] (placeholder for future melting)
  Real _tliq;            ///< Fuel liquidus temperature [K] (placeholder for future melting)
  Real _ufmelt;          ///< Fuel heat of fusion [J/kg] (placeholder for future melting)
  unsigned int _nrfuel;  ///< Number of fuel radial nodes
  
  std::vector<Real> _r;  ///< Radial node locations [m]
  std::vector<Real> _dr; ///< Radial control-volume widths [m]
  /** @} */

  /** @name Solution/state vectors @{ */
  std::vector<Real> _temp1; ///< Work vector for iterations [K]
  std::vector<Real> _temp;  ///< Current nodal temperatures [K]
  std::vector<Real> _temp0; ///< Previous-step temperatures [K]
  std::vector<Real> _qtrip; ///< Volumetric heat generation terms per node [W/m^3]
  /** @} */

  /** @name Linear system (per pin, per axial cell) @{ */
  std::vector<std::vector<Real>> _a; ///< Coefficient matrix storage (banded/tridiagonal form)
  std::vector<Real> _b;              ///< Right-hand side vector
  /** @} */

  /** @name Cached material properties at nodes @{ */
  std::vector<Real> _cpfuel;  ///< Fuel cp [J/kg/K]
  std::vector<Real> _cpclad;  ///< Clad cp [J/kg/K]
  std::vector<Real> _kfuel;   ///< Fuel k [W/m/K]
  std::vector<Real> _kclad;   ///< Clad k [W/m/K]
  std::vector<Real> _rhofuel; ///< Fuel density [kg/m^3]
  std::vector<Real> _rhoclad; ///< Clad density [kg/m^3]
  /** @} */

  /** @name Pin-averaged coolant fields (per axial, per pin) @{ */
  Eigen::ArrayXXd _tcool_pin_ave;    ///< Coolant average temperature [K]
  Eigen::ArrayXXd _pcool_pin_ave;    ///< Coolant average pressure [Pa]
  Eigen::ArrayXXd _hcool_pin_ave;    ///< Coolant average wall HTC [W/m^2/K]
  Eigen::ArrayXXd _qbarconv_channel; ///< Convective heat added to each channel [W]
  /** @} */

  /** @name Output fields @{ */
  /// Pin radial temperature profiles (per axial, per pin, per radial node) [K]
  Eigen::Array<Eigen::ArrayXXd, Eigen::Dynamic, 1> _temp_pin;
  /// Previous time value of _temp_pin
  Eigen::Array<Eigen::ArrayXXd, Eigen::Dynamic, 1> _temp_pin0;
  /** @} */
};
