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
// class LiquidMetalCDAP;
// class TriSubChannelMesh;
/**
 * Steady state subchannel solver for 1-phase hex liquid metal coolants
 */
class PinTempSolver : public TriSubChannel1PhaseProblem
{
public:
  PinTempSolver(const InputParameters & params);

  virtual ~PinTempSolver();

  //  virtual void initializeSolutionPinTempSolver() override;
  virtual void initializeSolutionPinTempSolver();

  /// flag for whether jet is alive or not
  std::vector<int> ijet;
  /// the rod number for the originating jet
  std::vector<int> rodjet;
  /// peak CDF axial location for the clad failure
  std::vector<int> peak_loc;
  /// jet heat transfer coefficient
  std::vector<Real> h_jet;
  /// jet temperature
  std::vector<Real> temp_jet;

  static InputParameters validParams();

protected:
  TriSubChannelMesh & _tri_sch_mesh;

  virtual void externalSolve() override;
  /// initialize at the beginning of the time step
  //  virtual void PinTempSolverDriver(Real dt, unsigned int pintemp_ss) override;
  virtual void PinTempSolverDriver(Real dt, unsigned int pintemp_ss);
  /// TempSolvCDAP
  void TempSolverTR(Real dt, unsigned int iz, unsigned int i_pin, Real hcoef, Real tcool);

  void TempSolverSS(unsigned int iz, unsigned int i_pin, Real hcoef, Real tcool);

  void set_convective_bc();

  Real computeAddedHeatPin(unsigned int i_ch, unsigned int iz) override;

  /// metal fuel thermal conductivity
  Real MetalFuelTHCON(Real temp, Real _wpu, Real _wzr, Real _por, Real ri);
  /// metal fuel specific heat
  Real MetalFuelCP(Real temp, Real _wpu, Real _wzr);
  /// metal fuel density
  Real MetalFuelRHO(Real _wpu, Real _wzr, Real _por);
  /// HT9 Cladding thermal conductivity
  Real HT9CladTHCON(Real temp);
  /// HT9 cladding Specific heat
  Real HT9CladCP(Real temp);
  /// Cladding density
  Real HT9CladRHO();
  /// initialize PinTempSolver
  unsigned int pintemp_init = 0;
  /// Steady-state integer flag, 0=SS, 1=TR
  unsigned int pintemp_ss = 0;

  Real _r0;
  Real _rfu;
  Real _rci;
  Real _hgap;
  Real _wpu;
  Real _wzr;
  Real _por;
  Real _tsol;
  Real _tliq;
  Real _ufmelt;
  unsigned int _nrfuel;
  unsigned int _nrpin;
  Real _rco;
  std::vector<Real> _r;
  std::vector<Real> _dr;

  std::vector<Real> _temp1;
  std::vector<Real> _temp;
  std::vector<Real> _temp0;
  std::vector<Real> _qtrip;

  std::vector<std::vector<Real>> _a;

  std::vector<Real> _b;

  std::vector<Real> _cpfuel;
  std::vector<Real> _cpclad;
  std::vector<Real> _kfuel;
  std::vector<Real> _kclad;
  std::vector<Real> _rhofuel;
  std::vector<Real> _rhoclad;

  /// coolant average temperature per pin
  Eigen::ArrayXXd _tcool_pin_ave;
  /// coolant average pressure per pin
  Eigen::ArrayXXd _pcool_pin_ave;
  /// coolant average heat transfer coefficient per pin
  Eigen::ArrayXXd _hcool_pin_ave;
  /// convective heat transfer to the coolant channel
  Eigen::ArrayXXd _qbarconv_channel;

  Eigen::Array<Eigen::ArrayXXd, Eigen::Dynamic, 1> _temp_pin;
  // Matrix <Real, 3,3,3> matrixA;
};
