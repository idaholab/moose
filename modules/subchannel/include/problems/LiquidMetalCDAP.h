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

#include "PinTempSolver.h"
#include <memory>
#include <Eigen/Dense>

class LiquidMetalCDAP;
class TriSubChannelMesh;
/**
 * Steady state subchannel solver for 1-phase hex liquid metal coolants
 */
class LiquidMetalCDAP : public PinTempSolver
{
public:
  LiquidMetalCDAP(const InputParameters & params);

  virtual ~LiquidMetalCDAP();

  virtual void initializeSolutionCDAP();
    /// compute CDAP
  virtual void executeCDAP(Real dt);

	void computeTrackChannelFG(Real dt);
	void computeChannelXAlphaFric();
	void computeInletMassflowFg();
	void computeHTfg();
	void computeFailZ(int i_rod, unsigned int peakz);
	void postfail_driver(Real dt);

protected:

  
  TriSubChannelMesh & _tri_sch_mesh;

// --- New breach/geometry/voiding helpers (pass numeric inputs; no mesh types) ---
// 1) Build breach unit vector & breach location on the rod OD
void computeBreachVector(Real xchan, Real ychan,
                         Real xrod,  Real yrod);


// 2) Angle (deg) between breach vector and a channel center (2D)
Real compute_angle2D(Real xchan, Real ychan) const;

// 3) Two-phase equivalent radius at an axial location zloc (m)
//    zfail = failure axial location (m); theta_deg from compute_angle2D
Real compute2pRadius(Real zloc, Real zfail, Real theta_deg) const;

// 4) Mark “intense” FG region for one axial row and form HT multipliers
//    Ax_row[j][i] style is common; pass the *row* j as a span/vector.
//    Outputs: fills InodeFG_Intense_row (0/1) and returns alpha_intense_j.
void computeIntensifiedVoiding();

//
Real _radius_frac;    
Real _deqFail;
Real _breach_diameter_ref;
Real _pgas_to_coolant_ratio_ref;
Real _r_oil;     // m
Real _m_oil;       // slope [m/m]
Real _max_two_phase_radius; // m
/// initial plenum pressure
Real _plegas0;
/// initial plenum temperature
Real _Tplegas0;
/// plenum to fuel ratio
Real _ptof_ratio;
/// upper limit of CDF
Real _cdfuplim;
/// molar weight of Argon
Real _mgas;
/// input jet length
Real _jetLength;
/// mean value of failure probability/CDF normal distribution
Real _cdfmean;
/// sigma of failure probability/CDF normal distribution
Real _cdfsigma;
    
Real _mu_oil = 3.4e-3, _mu_na = 2.4e-4;
Real _sigma_oil = 0.023, _sigma_na = 0.18;

// ---------- Minimal state this module keeps -----------
Real _breach_vec_x = 1.0, _breach_vec_y = 0.0;  // unit vector
Real _breach_loc_x = 0.0, _breach_loc_y = 0.0;  // breach point on OD
Real _breach_vec_mag = 1.0;                     // magnitude (pre-normalization)
std::vector<Real> _HT_FG;
std::vector<int> _inodeFG;
Real _mdot_ratio;
unsigned int _channel_fg = -1;
// ---------- Model parameters used by the helpers ----------




std::vector<Real> _p_ratio; //




Real _alpha;
Real _fric_mult;
Real _grav_mult;


  /// HT9 time to rupture due to thermal creep - Transient CDF Model
  Real trupt1(Real tcdot, Real stress, Real Tclad);

  /// HT9 time to rupture due to thermal creep - Steady State CDF Model
  Real trups1(Real stress, Real Tclad);

  /// HT9 time to rupture due to thermal creep - Combined CDF Model
  Real trupht9(Real tcdot, Real stress, Real Tclad);
  /// Calculate clad inner eutectic formation
  Real calc_deltaeut(Real temp);
  /// calculate fuel pin failure probability
  Real cdfnormaldist(Real peak_cdf);
  /// calculate the plenum pressure
  void calc_plenum_pressure(int i_rod, Real temp);
  //Calculate clad hoop stress
  Real calc_clad_stress(Real pgas, Real pcool, Real r_ci, Real r_co);
  /// compute the post-failure jet heat transfer
  void postfailjet(int i_rod, Real dt, Real pcool, Real Tcool, Real TgasPl, Real rho_na, Real axial_vel);
  /// initialize at the beginning of the time step
  virtual void initialize_timestepCDAP(Real dt);
  /// Calculate the sodium corrosion thickness
  Real calc_sodiumcorrosion();
  /// TempSolvCDAP
//  void TempSolvCDAP(Real dt, unsigned int iz, unsigned int i_pin, Real hcoef, Real tcool);

//  void TempSolvCDAPss(unsigned int iz, unsigned int i_pin, Real hcoef, Real tcool);

//  void set_convective_bc();

//  Real computeAddedHeatPin(unsigned int i_ch, unsigned int iz) override;
//  Real computeAddedHeatPin1(unsigned int i_ch, unsigned int iz);
//  Real computeAddedHeatPin2(unsigned int i_ch, unsigned int iz);

public:
  static InputParameters validParams();

protected:
  /// Computes mass flow per channel for block iblock
 virtual void computeMdot(int iblock) override;
  
 virtual void externalSolve() override;

    Real _r0;
    
    Real _rci;
    
    Real _rfu;
    
    int _nrfuel;


    /// total failure probability over all pins
    Real _total_prob;
    /// total failure probability over surviving pins
    Real _total_survivor_prob;
    /// total pin failures
    Real _total_fail;

    ///gas constant
    Real _rgas = 8.31446261815324;

    /// plenum volume
    Real _plenum_vol;
    /// CDAP time counter
    Real _time_st = 0.0;

	/// pressure offset or outlet pressure
//	Real p_offset = 2.0e+5;
	/// forced failure time
	Real _forcedFailTime = 2.0;
	/// Forced pin Failure integer flag
	int _forcedFail = 0;
	/// Forced failure pin number
	int _forcedPin = 1;


/**
    /// Experiment-1 Argon
    /// molar weight of Argon
    double mgas_ = 39.948e-03;
    /// input jet length
    double JetLength = 1.42e-3;
    /// input orifice equivalent diameter
    double DeqFail = 0.584e-03;
    /// Coolant pressure - Experiment
    double Pcool_exp = 4.0E+5;
    /// Coolant temperature - Experiment
    double Tcool_exp = 588.15;
    /// plenum gas temperature - Experiment
    double Tgas_exp = 783.15;
    ///plenum gas pressure - experiment
    double Pgas_exp = 5.0e+6;
    ///Plenum volume - experiment
    double PlenumVol_exp = 50.0e-6;
    ///coolant density - Experiment
    double rho_naexp = 874.0;
    /// coolant axial velocity - experiment
    double Coolant_velexp = 5.3;
    /// target heat flux W/m2
    double target_heat_flux = 1.26e+6;
**/ // end of experiment-1

/**
    /// Experiment-2 Argon
    /// molar weight of Argon
    double mgas_ = 39.948e-03;
    /// input jet length
    double JetLength = 1.42e-3;
    /// input orifice equivalent diameter
    double DeqFail = 0.584e-03;
    /// Coolant pressure - Experiment
    double Pcool_exp = 4.0E+5;
    /// Coolant temperature - Experiment
    double Tcool_exp = 588.15;
    /// plenum gas temperature - Experiment
    double Tgas_exp = 993.15;
    ///plenum gas pressure - experiment
    double Pgas_exp = 3.0e+6;
    ///Plenum volume - experiment
    double PlenumVol_exp = 50.0e-6;
    ///coolant density - Experiment
    double rho_naexp = 874.0;
    /// coolant axial velocity - experiment
    double Coolant_velexp = 5.3;
    /// target heat flux W/m2
    double target_heat_flux = 1.26e+6;
**/ // end of experiment-2

/**
    /// Experiment-3 Argon
    /// molar weight of Argon
    double mgas_ = 131.3e-03;
    /// input jet length
    double JetLength = 1.42e-3;
    /// input orifice equivalent diameter
    double DeqFail = 0.584e-03;
    /// Coolant pressure - Experiment
    double Pcool_exp = 4.0E+5;
    /// Coolant temperature - Experiment
    double Tcool_exp = 588.15;
    /// plenum gas temperature - Experiment
    double Tgas_exp = 783.15;
    ///plenum gas pressure - experiment
    double Pgas_exp = 3.0e+6;
    ///Plenum volume - experiment
    double PlenumVol_exp = 50.0e-6;
    ///coolant density - Experiment
    double rho_naexp = 874.0;
    /// coolant axial velocity - experiment
    double Coolant_velexp = 5.3;
    /// target heat flux W/m2
    double target_heat_flux = 1.26e+6;
**/ // end of experiment-3


    /// Experiment-4 Argon

    /// input orifice equivalent diameter
//    double DeqFail = 0.33e-03;
    /// Coolant pressure - Experiment
    Real _pcool_exp = 4.0E+5;
    /// Coolant temperature - Experiment
    Real _tcool_exp = 588.15;
    /// plenum gas temperature - Experiment
    Real _tgas_exp = 783.15;
    ///plenum gas pressure - experiment
    Real _pgas_exp = 5.0e+6;
    ///Plenum volume - experiment
    Real _plenumVol_exp = 50.0e-6;
    ///coolant density - Experiment
    Real _rho_naexp = 874.0;
    /// coolant axial velocity - experiment
    Real _coolant_velexp = 5.3;
    /// target heat flux W/m2
    Real _target_heat_flux = 1.26e+6;
// end of experiment-4

	/// input orifice diameter
//	Real DeqFail = 1.5e-3;
	/// molar weight of effective fission gas
//	Real mgas_ = 131.3e-3;
	/// loss coefficient at orifice
	Real _cd = 0.60;
	/// empirical jet constant
	Real _e0 = 0.08;
	/// critical jet deflection angle - experiment
	Real _crit_deflection_angle = 45.0;
	/// sodium specific heat J/kg/K - to be implemented as T dependent
	Real _cp_na = 1300.0;
	/// cp/cv ratio for ideal gas
	Real _gamma = 1.66;

    Real _axial_velexp = 1.0;

    /// failure probability per pin
    std::vector<Real> _fail_prob;
    /// plenum gas pressure
    std::vector<Real> _pgas;

    /// clad inner wastage
    Eigen::ArrayXXd _clad_wastage_in;
    Eigen::ArrayXXd _clad_wastage_in_old;
    /// clad outer wastage
    Eigen::ArrayXXd _clad_wastage_out;
    Eigen::ArrayXXd _clad_wastage_out_old;
    /// clad hoop stress
    Eigen::ArrayXXd _stress;
	/// fuel pin temperatures
//	Eigen::ArrayXXd temp;

    /// peak cdf
    std::vector<Real> _peak_cdf;
    /// clad rupture time
//    std::vector<std::vector<double>> clad_trup;
    Eigen::ArrayXXd _clad_trup;

    /// pin failure flag
    std::vector<int> _ifail;
    /// neighbor channels per pin
    std::vector<int> _neig_chan;
    /// channel fractions per pin
    std::vector<Real> _chan_frac;
    /// failure channel per pin
    std::vector<int> _chan_fail;
    /// time at which jet is initiated
    std::vector<Real> _time_ijet;
    /// remaining fission gas mass in plenum
    std::vector<Real> _mfg;
    /// two phase mass flow rate
    std::vector<Real> _mdote;
    /// critical pressure
    std::vector<Real> _pst_crit;
    /// pin pressure
    std::vector<Real> _pst;



//  	Eigen::Array<Eigen::ArrayXXd, Eigen::Dynamic, 1> temp_pin;
    //Matrix <Real, 3,3,3> matrixA;


    /// Clad cumulative damage fraction
    Eigen::ArrayXXd _clad_cdf;

    /// Clad cumulative damage fraction, previous timestep
    Eigen::ArrayXXd _clad_cdf_old;

	std::vector<Real> _mdotFG_ej;
	std::vector<Real> _trailFG;

	Real _mdotFG_chan_ave;

	std::vector<Real> _lengthz_FailFG;
	std::vector<Real> _z_FailFG;
	std::vector<Real> _rhoFG;
	
	Real _lowz_boundFG;
	Real _highz_boundFG;
	Real _fully_developed_lowz;
	Real _fully_developed_highz;
	int _timestep_counterFG;
	int _timestep_counterFG0;
	Real _GRAV = 9.81;
	
	Real _mdotfg_tot;
	unsigned int _peak_loc_fg;
	unsigned int _irod_fg;

	std::vector<unsigned int> _origin_rod;



    Eigen::ArrayXXd _HTFG_IntenseChan;
    Eigen::ArrayXXi _inodeFG_Intense;
    std::vector<Real> _alpha_intense;
    
    // Axial spacing (cell thickness) and cumulative edge positions (relative to z0)
// Sizes: _dz has length _n_cells+1 (index 0 unused by convention here);
//        _zloc has length _n_cells+1 with _zloc[0] = 0 and _zloc[j] = Σ_{k=1..j} _dz[k].
std::vector<Real> _dz;    // valid entries: j = 1.._n_cells
std::vector<Real> _zloc;  // valid entries: j = 0.._n_cells (edges), with _zloc[0] = 0

    
    
};
