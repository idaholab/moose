//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MultiMooseEnum.h"
#include "PertinentGeochemicalSystem.h"
#include "GeochemistrySpeciesSwapper.h"
#include "GeochemistryActivityCoefficientsDebyeHuckel.h"
#include "GeochemistryConstants.h"
#include "GeochemistryUnitConverter.h"

/**
 * This class holds information about bulk composition, molalities, activities, activity
 * coefficients, etc of the user-defined geochemical system in PertinentGeochemicalSystem.  Hence,
 * it extends the generic information in PertinentGeochemicalSystem to a system that is specific to
 * one spatio-temporal location.  It offers methods to manipulate these molalities, bulk
 * compositions, etc, as well as performing basis swaps.
 *
 * Related to this, this class also builds the so-called "algebraic system" which is the nonlinear
 * system of algebraic equations for the unknown basis molalities, surface potentials and mole
 * number of kinetic species, and computes the residual vector and jacobian of this algebraic
 * system.  This class initialises the algebraic variables (the unknown molalities, surface
 * potentials and kinetic moles) to reaponable values, but it does not solve the algebraic system:
 * instead, it has a "setter" method, setAlgebraicValues that can be used to set the unknowns, hence
 * another class can use whatever method it desires to solve the system.
 *
 * WARNING: because GeochemicalSystem performs swaps, it will change the
 * ModelGeochemicalDatabase object.
 * WARNING: because GeochemicalSystem sets internal parameters in the
 * activity-coefficient object, it will change the GeochemistryActivityCoefficient object.
 */
class GeochemicalSystem
{
public:
  /**
   * Each basis species has one of the following constraints.  During the process of units
   * conversion (from user-prescribed units to mole-based units) each ConstraintMeaningUserEnum
   * supplied by the user is translated to the appropriate ConstraintMeaningEnum
   */
  enum class ConstraintMeaningEnum
  {
    MOLES_BULK_WATER,
    KG_SOLVENT_WATER,
    MOLES_BULK_SPECIES,
    FREE_MOLALITY,
    FREE_MOLES_MINERAL_SPECIES,
    FUGACITY,
    ACTIVITY
  };

  /// Each basis species must be provided with a constraint, that is chosen by the user from the following enum
  enum class ConstraintUserMeaningEnum
  {
    KG_SOLVENT_WATER,
    BULK_COMPOSITION,
    BULK_COMPOSITION_WITH_KINETIC,
    FREE_CONCENTRATION,
    FREE_MINERAL,
    ACTIVITY,
    LOG10ACTIVITY,
    FUGACITY,
    LOG10FUGACITY
  };

  /**
   * Construct the geochemical system, check consistency of the constructor's arguments,
  and initialize all internal variables (molalities, bulk compositions, equilibrium constants,
  activities, activity coefficients, surface potentials and kinetic mole numbers) and set up the
  algebraic system
   * @param mgd the model's geochemical database, which is a model-specific version of the full
  geochemical database.  WARNING: because GeochemicalSystem performs swaps, it will
  change mgd
  * @param gac the object that computes activity coefficients.  WARNING: because
  GeochemicalSystem sets internal parameters in gac, it will change it.
  * @param is object to compute ionic strengths
  * @param swapper object to perform swaps on mgd
   * @param swap_out_of_basis A list of basis species that should be swapped out of the basis and
  replaced with swap_into_basis, prior to any other computations
   * @param swap_into_basis A list of equilibrium species that should be swapped into the basis in
  place of swap_out_of_basis
   * @param charge_balance_species The charge-balance species, which must be a basis species after
  the swaps are performed
   * @param constrained_species A list of the basis species after the swaps have been performed
   * @param constraint_values Numerical values of the constraints placed on the basis species.  Each
  basis species must have exactly one constraint
   * @param constraint_unit Units of the constraint_values.  Each constraint_value must have
  exactly one constraint_unit.  This is only used during construction, to convert the
  constraint_values to mole units
   * @param constraint_user_meaning A list the provides physical meaning to the constraint_values
   * @param initial_temperature Initial temperature
   * @param iters_to_make_consistent The initial equilibrium molalities depend on the activity
  coefficients, which depend on the basis and equilibrium molalities.  This circular dependence
  means it is usually impossible to define an exactly consistent initial configuration for
  molalities.  An iterative process is used to approach the consistent initial configuration, using
  iters_to_make_consistent iterations.  Usually iters_to_make_consistent=0 is reasonable, because
  there are so many approximations used in the solution process that a slightly inconsistent initial
  configuration is fine
   * @param min_initial_molality Minimum value of equilibrium molality used in the initial condition
   * @param kin_name Names of the kinetic species that are provided with initial conditions in
  kin_initial_moles.  All kinetic species must be provided with an initial condition
   * @param kin_initial Values of the initial mole number or mass or volume (depending on
  kin_unit) for the kinetic species
   * @param kin_unit The units of the numbers given in kin_initial
   */
  GeochemicalSystem(ModelGeochemicalDatabase & mgd,
                    GeochemistryActivityCoefficients & gac,
                    GeochemistryIonicStrength & is,
                    GeochemistrySpeciesSwapper & swapper,
                    const std::vector<std::string> & swap_out_of_basis,
                    const std::vector<std::string> & swap_into_basis,
                    const std::string & charge_balance_species,
                    const std::vector<std::string> & constrained_species,
                    const std::vector<Real> & constraint_value,
                    const MultiMooseEnum & constraint_unit,
                    const MultiMooseEnum & constraint_user_meaning,
                    Real initial_temperature,
                    unsigned iters_to_make_consistent,
                    Real min_initial_molality,
                    const std::vector<std::string> & kin_name,
                    const std::vector<Real> & kin_initial_moles,
                    const MultiMooseEnum & kin_unit);

  GeochemicalSystem(
      ModelGeochemicalDatabase & mgd,
      GeochemistryActivityCoefficients & gac,
      GeochemistryIonicStrength & is,
      GeochemistrySpeciesSwapper & swapper,
      const std::vector<std::string> & swap_out_of_basis,
      const std::vector<std::string> & swap_into_basis,
      const std::string & charge_balance_species,
      const std::vector<std::string> & constrained_species,
      const std::vector<Real> & constraint_value,
      const std::vector<GeochemistryUnitConverter::GeochemistryUnit> & constraint_unit,
      const std::vector<ConstraintUserMeaningEnum> & constraint_user_meaning,
      Real initial_temperature,
      unsigned iters_to_make_consistent,
      Real min_initial_molality,
      const std::vector<std::string> & kin_name,
      const std::vector<Real> & kin_initial,
      const std::vector<GeochemistryUnitConverter::GeochemistryUnit> & kin_unit);

  /**
   * Copy assignment operator.  Almost all of the following is trivial.  The most important
   * non-trivial feature is copying src._mgd into our _mgd.  Note this method gets called when
   * dest = src
   * and dest is already constructed.  (Code such as GeochemicalSystem dest = src uses the copy
   * constructor which simply sets the _mgd reference in dest equal to the _mgd reference in src,
   * and does not make a copy of the data within src._mgd)
   */
  GeochemicalSystem & operator=(const GeochemicalSystem & src)
  {
    if (this == &src) // trivial a=a situation
      return *this;
    // check for bad assignment situations.  Other "const" things that
    // needn't be the same (but probably actually are) include: _swap_out and _swap_in
    if (_num_basis != src._num_basis || _num_eqm != src._num_eqm || _num_redox != src._num_redox ||
        _num_surface_pot != src._num_surface_pot || _num_kin != src._num_kin ||
        _original_charge_balance_species != src._original_charge_balance_species)
      mooseError("GeochemicalSystem: copy assigment operator called with inconsistent fundamental "
                 "properties");
    // actually do the copying
    _mgd = src._mgd;
    _swapper = src._swapper;
    _gac = src._gac;
    _is = src._is;
    _charge_balance_species = src._charge_balance_species;
    _charge_balance_basis_index = src._charge_balance_basis_index;
    _constrained_species = src._constrained_species;
    _constraint_value = src._constraint_value;
    _original_constraint_value = src._original_constraint_value;
    _constraint_unit = src._constraint_unit;
    _constraint_user_meaning = src._constraint_user_meaning;
    _constraint_meaning = src._constraint_meaning;
    _eqm_log10K = src._eqm_log10K;
    _redox_log10K = src._redox_log10K;
    _kin_log10K = src._kin_log10K;
    _num_basis_in_algebraic_system = src._num_basis_in_algebraic_system;
    _num_in_algebraic_system = src._num_in_algebraic_system;
    _in_algebraic_system = src._in_algebraic_system;
    _algebraic_index = src._algebraic_index;
    _basis_index = src._basis_index;
    _bulk_moles_old = src._bulk_moles_old;
    _basis_molality = src._basis_molality;
    _basis_activity_known = src._basis_activity_known;
    _basis_activity = src._basis_activity;
    _eqm_molality = src._eqm_molality;
    _basis_activity_coef = src._basis_activity_coef;
    _eqm_activity_coef = src._eqm_activity_coef;
    _eqm_activity = src._eqm_activity;
    _surface_pot_expr = src._surface_pot_expr;
    _sorbing_surface_area = src._sorbing_surface_area;
    _kin_moles = src._kin_moles;
    _kin_moles_old = src._kin_moles_old;
    _iters_to_make_consistent = src._iters_to_make_consistent;
    _temperature = src._temperature;
    _min_initial_molality = src._min_initial_molality;
    _original_redox_lhs = src._original_redox_lhs;
    return *this;
  };

  /**
   * Copy constructor.
   */
  GeochemicalSystem(const GeochemicalSystem & src) = default;

  bool operator==(const GeochemicalSystem & rhs) const
  {
    return (_mgd == rhs._mgd) && (_num_basis == rhs._num_basis) && (_num_eqm == rhs._num_eqm) &&
           (_num_redox == rhs._num_redox) && (_num_surface_pot == rhs._num_surface_pot) &&
           (_num_kin == rhs._num_kin) && (_swapper == rhs._swapper) &&
           (_swap_out == rhs._swap_out) && (_swap_in == rhs._swap_in) && (_gac == rhs._gac) &&
           (_is == rhs._is) && (_charge_balance_species == rhs._charge_balance_species) &&
           (_original_charge_balance_species == rhs._original_charge_balance_species) &&
           (_charge_balance_basis_index == rhs._charge_balance_basis_index) &&
           (_constrained_species == rhs._constrained_species) &&
           (_constraint_value == rhs._constraint_value) &&
           (_original_constraint_value == rhs._original_constraint_value) &&
           (_constraint_unit == rhs._constraint_unit) &&
           (_constraint_user_meaning == rhs._constraint_user_meaning) &&
           (_constraint_meaning == rhs._constraint_meaning) && (_eqm_log10K == rhs._eqm_log10K) &&
           (_redox_log10K == rhs._redox_log10K) && (_kin_log10K == rhs._kin_log10K) &&
           (_num_basis_in_algebraic_system == rhs._num_basis_in_algebraic_system) &&
           (_num_in_algebraic_system == rhs._num_in_algebraic_system) &&
           (_in_algebraic_system == rhs._in_algebraic_system) &&
           (_algebraic_index == rhs._algebraic_index) && (_basis_index == rhs._basis_index) &&
           (_bulk_moles_old == rhs._bulk_moles_old) && (_basis_molality == rhs._basis_molality) &&
           (_basis_activity_known == rhs._basis_activity_known) &&
           (_basis_activity == rhs._basis_activity) && (_eqm_molality == rhs._eqm_molality) &&
           (_basis_activity_coef == rhs._basis_activity_coef) &&
           (_eqm_activity_coef == rhs._eqm_activity_coef) && (_eqm_activity == rhs._eqm_activity) &&
           (_surface_pot_expr == rhs._surface_pot_expr) &&
           (_sorbing_surface_area == rhs._sorbing_surface_area) && (_kin_moles == rhs._kin_moles) &&
           (_kin_moles_old == rhs._kin_moles_old) &&
           (_iters_to_make_consistent == rhs._iters_to_make_consistent) &&
           (_temperature == rhs._temperature) &&
           (_min_initial_molality == rhs._min_initial_molality) &&
           (_original_redox_lhs == rhs._original_redox_lhs);
  }

  /// returns the number of species in the basis
  unsigned getNumInBasis() const;

  /// returns the number of species in equilibrium with the basis components
  unsigned getNumInEquilibrium() const;

  /// returns the number of kinetic species
  unsigned getNumKinetic() const;

  /// initialize all variables, ready for a Newton solve of the algebraic system
  void initialize();

  /// return the index of the charge-balance species in the basis list
  unsigned getChargeBalanceBasisIndex() const;

  /**
   * If the molality of the charge-balance basis species is less than threshold molality, then
   * attempt to change the charge-balance species, preferably to another component with opposite
   * charge and big molality.
   * @param threshold_molality if the charge-balance basis species has molality greater than this,
   * this routine does nothing
   * @return true if the charge-balance species was changed
   */
  bool alterChargeBalanceSpecies(Real threshold_molality);

  /**
   * @param j the index of the equilibrium species
   * @return the value of log10(equilibrium constant) for the j^th equilibrium species
   */
  Real getLog10K(unsigned j) const;

  /**
   * @return the number of redox species in disequibrium
   */
  unsigned getNumRedox() const;

  /**
   * @param red the index of the redox species in disequilibrium (red < _num_redox)
   * @return the value of log10(equilibrium constant) for the given disequilibrium-redox species
   */
  Real getRedoxLog10K(unsigned red) const;

  /**
   * @param red the index of the redox species in disequilibrium (red < _num_redox)
   * @return the value of log10(activity product) for the given disequilibrium-redox species
   */
  Real log10RedoxActivityProduct(unsigned red) const;

  /**
   * @param kin the index of the kinetic species (must be < _num_kin)
   * @return the value of log10(equilibrium constant) for the given kinetic species
   */
  Real getKineticLog10K(unsigned kin) const;

  /**
   * @return the value of log10(equilibrium constant) for the each kinetic species
   */
  const std::vector<Real> & getKineticLog10K() const;

  /**
   * @param kin the index of the kinetic species (must be < _num_kin)
   * @return the value of log10(activity product) for the given kinetic species
   */
  Real log10KineticActivityProduct(unsigned kin) const;

  /**
   * @param kin the index of the kinetic species (must be < _num_kin)
   * @return the mole number for the given kinetic species
   */
  Real getKineticMoles(unsigned kin) const;

  /**
   * Sets the current AND old mole number for a kinetic species.  Note: this does not compute a
   * consistent configuration (viz, the bulk mole composition is not updated): use
   * computeConsistentConfiguration if you need to.
   * @param kin the index of the kinetic species (must be < _num_kin)
   * @param moles the number of moles
   */
  void setKineticMoles(unsigned kin, Real moles);

  /**
   * @return the mole number for all kinetic species
   */
  const std::vector<Real> & getKineticMoles() const;

  /**
   * Usually used at the end of a solve, to provide correct initial conditions to the next
   * time-step, this method:
   * - sets kin_moles_old = kin_moles
   * - updates the constraint_values and bulk_moles_old with mole_additions, for BULK-type species
   * - ensures charge balance holds if all species are BULK-type species
   * - computes basis_molality for the BULK-type mineral species
   * - computes bulk_moles_old from the molalities for non-BULK-type species
   * Note that it is possible that you would like to enforceChargeBalance() immediately after
   * calling this method: that is fine, there will be no negative consequences if the system has
   * been solved
   * @param mole_additions the increment of mole number of each basis species and kinetic species
   * since the last timestep.  This must have size _num_basis + _num_kin.  Only the first _num_basis
   * of these are used.
   */
  void updateOldWithCurrent(const DenseVector<Real> & mole_additions);

  /**
   * @return the number in the algebraic system (number of basis species in algebraic system +
   * number of surface potentials + number of kinetic species)
   */
  unsigned getNumInAlgebraicSystem() const;

  /// return the number of basis species in the algebraic system
  unsigned getNumBasisInAlgebraicSystem() const;

  /// return the number of surface potentials
  unsigned getNumSurfacePotentials() const;

  /**
   * @return a vector of length _num_basis whose entries determine whether the basis species is in
   * the algebraic system
   */
  const std::vector<bool> & getInAlgebraicSystem() const;

  /**
   * @return v, a vector of length _num_basis, where v[i] = the basis index of the i^th species in
   the algebraic system.  Note that for i > _num_basis_in_algebraic_system, the value of v is
   undefined.
   */
  const std::vector<unsigned> & getBasisIndexOfAlgebraicSystem() const;

  /**
   * @return v, a vector of length _num_basis, where v[i] = the algebraic index of the i^th basis
   * species.  Note that unless the i^th basis species is in the algebraic system, this is
   * meaningless
   */
  const std::vector<unsigned> & getAlgebraicIndexOfBasisSystem() const;

  /**
   * @return v, a vector of length getNumInAlgebraicSystem, the elements of which are the current
   * values of the algebraic variables
   */
  std::vector<Real> getAlgebraicVariableValues() const;

  /**
   * @return v, a vector of length _num_basis_in_algebraic_system, the elements of which are the
   * current values of the algebraic variables: molalities only (not surface potentials or kinetic
   * species)
   */
  std::vector<Real> getAlgebraicBasisValues() const;

  /**
   * @return v, a DenseVector of length getNumInAlgebraicSystem, the elements of which are the
   * current values of the algebraic variables
   */
  DenseVector<Real> getAlgebraicVariableDenseValues() const;

  /// Returns the mass of solvent water
  Real getSolventWaterMass() const;

  /**
   * @return the number of bulk-composition moles.  Note this will typically be the "old" number of
   * bulk moles (from the previous time-step) unless addtoBulkMoles or updateOldWithCurrent or other
   * similar methods have just been called.  Note that this contains contributions from kinetic
   * species, in contrast to the Bethke approach.
   */
  const std::vector<Real> & getBulkMolesOld() const;

  /**
   * @return the number of bulk-composition moles in the original basis.  Note this will typically
   * be the "old" number of bulk moles (from the previous time-step) unless addtoBulkMoles or
   * updateOldWithCurrent or other similar methods have just been called.  Note that this contains
   * contributions from kinetic species, in contrast to the Bethke approach.
   */
  DenseVector<Real> getBulkOldInOriginalBasis() const;

  /**
   * @return the number of bulk-composition moles that are transported in reactive-transport,
   * expressed in the original basis.  Note this is computed using the existing molalities, so the
   * result might be junk if the system is inconsistent, but will be OK if, for instance, a solve
   * has just converged.  Also note that this does not include contributions from kinetic species
   */
  DenseVector<Real> getTransportedBulkInOriginalBasis() const;

  /**
   * Computes the value of transported bulk moles for all basis species using the existing
   * molalities.  Note that this is probably gives rubbish results unless the system is consistent
   * (eg, the solve has converged).  Also note that this does not include contributions from kinetic
   * species
   */
  void computeTransportedBulkFromMolalities(std::vector<Real> & transported_bulk) const;

  /**
   * @return vector v, where v[i] = mass of solvent water (i=0), or v[i] = molality of the basis
   * aqueous species i, or v[i] = free moles of basis mineral i, whichever is appropriate
   */
  const std::vector<Real> & getSolventMassAndFreeMolalityAndMineralMoles() const;

  /**
   * @return vector v, where v[i] = true if the activity of basis species i is fixed, either by the
   * user providing an activity value, or a fugacity value, or because the i^th basis species is a
   * mineral
   */
  const std::vector<bool> & getBasisActivityKnown() const;

  /**
   * @return the activity for the i^th basis species
   */
  Real getBasisActivity(unsigned i) const;

  /**
   * @return the activity for the basis species
   */
  const std::vector<Real> & getBasisActivity() const;

  /**
   * @return the molality of the j^th equilibrium species.  This is not defined for minerals or
   * gases
   */
  Real getEquilibriumMolality(unsigned j) const;

  /**
   * @return the molalities of the equilibrium species.  These are not defined for minerals or
   * gases
   */
  const std::vector<Real> & getEquilibriumMolality() const;

  /**
   * @return the activity coefficient for the i^th basis species.  This is not defined for water,
   * minerals, gases, or aqueous species that have been provided an activity by the user
   */
  Real getBasisActivityCoefficient(unsigned i) const;

  /**
   * @return the activity coefficients for the basis species.  These are not defined for water,
   * minerals, gases, or aqueous species that have been provided an activity by the user
   */
  const std::vector<Real> & getBasisActivityCoefficient() const;

  /**
   * @return the activity coefficient for the j^th equilibrium species.  This is not defined for
   * minerals
   */
  Real getEquilibriumActivityCoefficient(unsigned j) const;

  /**
   * @return the activity coefficients for the equilibrium species.  These are not defined for
   * minerals
   */
  const std::vector<Real> & getEquilibriumActivityCoefficient() const;

  /**
   * @return the total charge in the system.  Note this is based on the old values of bulk moles and
   * kinetic moles (ie, from the previous time-step) so to get the current values methods like
   * addToBulkMoles should be called, or updateOldWithCurrent
   */
  Real getTotalChargeOld() const;

  /**
   * Return the residual of the algebraic system for the given algebraic index
   * @param algebraic_ind the algebraic index
   * @param mole_additions the increment of mole number of each basis species and kinetic species
   * since the last timestep.  This must have size _num_basis + _num_kin.  For the basis species,
   * this is the amount of each species being injected into the system over the timestep.  For the
   * kinetic species, this is -dt*reaction_rate.  Please note: do not decompose the kinetic-species
   * additions into basis components and add them to the first slots of mole_additions!  This method
   * does that decomposition automatically.  The first _num_basis slots of mole_additions contain
   * kinetic-independent additions, while the last _num_kin slots contain kinetic-rate
   * contributions.
   */
  Real getResidualComponent(unsigned algebraic_ind, const DenseVector<Real> & mole_additions) const;

  /**
   * @return reference to the underlying ModelGeochemicalDatabase
   */
  const ModelGeochemicalDatabase & getModelGeochemicalDatabase() const;

  /**
   * Copies a ModelGeochemicalDatabase into our _mgd structure
   * @param mgd reference to the ModelGeochemicalDatabase that will be copied into _mgd
   */
  void setModelGeochemicalDatabase(const ModelGeochemicalDatabase & mgd);

  /**
   * Compute the Jacobian for the algebraic system and put it in jac
   * @param res The residual of the algebraic system.  This is used to speed up computations of the
   * jacobian
   * @param jac The jacobian entries are placed here
   * @param mole_additions The molar additions to the basis species and the kinetic species
   * @param dmole_additions d(mole_additions)/d(molality or kinetic_moles)
   */
  void computeJacobian(const DenseVector<Real> & res,
                       DenseMatrix<Real> & jac,
                       const DenseVector<Real> & mole_additions,
                       const DenseMatrix<Real> & dmole_additions) const;

  /**
   * Set the variables in the algebraic system (molalities and potentially the mass of solvent
   * water, and surface potentials, and kinetic mole numbers if any) to algebraic_var.  All elements
   * of this vector must be postivie.  This function also calls computeConsistentConfiguration.
   * @param algebraic_var the values to set the algebraic variables to
   */
  void setAlgebraicVariables(const DenseVector<Real> & algebraic_var);

  /**
   * Enforces charge balance by altering the constraint_value and bulk_moles_old of the
   * charge-balance species.  Use with caution, since this overwrites the constraint values provided
   * in the constructor, and also changes bulk_moles_old which is usually the value of bulk moles
   * from the previous time-step
   */
  void enforceChargeBalance();

  /**
   * Changes the charge-balance species to the original that is specified in the constructor (this
   * might not be possible since the original charge-balance species might have been swapped out)
   * @return true if the charge-balance species is changed
   */
  bool revertToOriginalChargeBalanceSpecies();

  /**
   * @return vector v, where v[j] = saturation index of equilibrium species j = log10(activity
   * product) - log10K.  If the equilibrium species is not a mineral then v[j] = 0
   */
  std::vector<Real> getSaturationIndices() const;

  /**
   * Perform the basis swap, and ensure that the resulting system is consistent.  Note that this
   * alters the bulk_moles_old of species.  If you are confident that your configuration is
   * reasonably correct, you should therefore ensure bulk_moles_old is set to the actual bulk_moles
   * in your system prior to calling performSwap.  Alternatively, if you are unsure of the
   * correctness, just let performSwap use bulk_moles_old to set the bulk moles of your new basis.
   * @param swap_out_of_basis index of basis species to remove from basis
   * @param swap_into_basis index of equilibrium species to add to basis
   */
  void performSwap(unsigned swap_out_of_basis, unsigned swap_into_basis);

  /// Get the ionic strength
  Real getIonicStrength() const;

  /// Get the stoichiometric ionic strength
  Real getStoichiometricIonicStrength() const;

  /**
   * @param sp surface number of interest.  sp < _num_surface_pot
   * @return the surface potential (units Volts) for the surface number
   */
  Real getSurfacePotential(unsigned sp) const;

  /**
   * @param sp surface number of interest.  sp < _num_surface_pot
   * @return the specific charge of the surface (units: Coulombs/m^2) for the surface number
   */
  Real getSurfaceCharge(unsigned sp) const;

  /**
   * @return the sorbing surface area (units: m^2) for each sorbing surface
   */
  const std::vector<Real> & getSorbingSurfaceArea() const;

  /**
   * @return the temperature in degC
   */
  Real getTemperature() const;

  /**
   * Sets the temperature to the given quantity.  This also:
   * - calculates new values of equilibrium constants for equilibrium, redox and kinetic species
   * - instructs the activity-coefficient calculator to set its internal temperature and calculate
   * new values of the Debye-Huckel parameters.
   * However, it does NOT update activity coefficients, basis molalities for species of known
   * activities, basis activities, equilibrium molalities, bulk mole number of species with fixed
   * free molality, free mineral mole numbers, or surface sorbing area.  Hence, the state of the
   * equilibrium geochemical system will be left inconsistent after a call to setTemperature, and
   * you should computeConsistentConfiguration() to rectify this, if needed.
   * @param temperature the temperature in degC
   */
  void setTemperature(Real temperature);

  /**
   * Sets:
   * - solvent water mass
   * - free molality of all basis species and equilibrium species that are not (water or gas or
   * mineral)
   * - free number of moles of all minerals (if any)
   * - surface potential expressions for all surface-sorption sites (if any)
   * - mole number and old mole number of the kinetic species (if any)
   * Then computes bulk mole composition, activities, activity coefficients and sorbing surface
   * areas so that the GeochemicalSystem is kept self-consistent.
   *
   * This method is designed to be run at the start of a simulation, or during a "recover" operator.
   *
   * @param names the names of all the basis species, equilibrium species, surface-sorption
   * minerals and kinetic species that are in the system.  Note, there must be exactly num_basis +
   * num_eqm + num_surface_pot + num_kin of these, ie, all species must be provided with a value
   * @param values the values of the molalities (or solvent water mass for water, or free number of
   * moles for minerals/kinetics, or surface_potential_expr for surface-sorption sites).  There must
   * be an equal number of these as "names".
   * @param contraints_from_molalities whether the constraints initial provided to the
   * GeochemicalSystem should be updated by the values provided.  This must have size
   * num_basis.
   *
   * This method is reasonably nontrivial:
   * - it assumes that temperature-dependent quantities (equilibrium constants, Debye-Huckel
   * parameters, etc) have been set prior to calling this method, eg in the constructor or
   * setTemperature()
   * - it assumes the algebraic info has been built during instantiation, or during swaps, etc
   * - it does not assume the value provided are "good", although since they most usually come from
   * a previous solve, they are typically pretty "good".  It does check for non-negativity: molality
   * for basis gases must be zero; molality for basis minerals must be non-negative; molality for
   * every other basis species must be positive; molality for equilibrium gases and equilibrium
   * minerals are set to zero irrespective of values provided; molality of other equilibrium species
   * must be non-negative; surface_potential_expressions must be positive; mole number of kinetic
   * species must be positive.  (In the previous sentence, "molality" is molality, kg solvent water,
   * or free mineral moles, whichever is appropriate.)
   * - if constraints_from_molalities is false then: if the original constraint was
   * KG_SOLVENT_WATER, FREE_MOLALITY or FREE_MOLES_MINERAL_SPECIES then the constraint take
   * precedence over the molality provided to this method, so the molality is ignored and the
   * constraint value used instead.
   * - if constraints_from_molalities is true then: if the original constraint was KG_SOLVENT_WATER,
   * FREE_MOLALITY or FREE_MOLES_MINERAL_SPECIES then the constraint value is set to the value
   * provided by to this method; if the original constraint was MOLES_BULK_WATER or
   * MOLES_BULK_SPECIES then the constraint value is set to the value computed from the molalities
   * provided to this method; if the original constraint was ACTIVITY, then the constraint value is
   * set to activity_coefficient * molality_provided_to_this_method
   * - Note the possibilities for ignoring the values provided to this method mentioned in the
   * preceeding paragraphs (setting to zero for equilibrium minerals and gases, and constraints
   * overriding the values provided)!
   */
  void setSolventMassAndFreeMolalityAndMineralMolesAndSurfacePotsAndKineticMoles(
      const std::vector<std::string> & names,
      const std::vector<Real> & values,
      const std::vector<bool> & constraints_from_molalities);

  /**
   * @return a vector of the constraint meanings for the basis species
   */
  const std::vector<GeochemicalSystem::ConstraintMeaningEnum> & getConstraintMeaning() const;

  /**
   * Changes a KG_SOLVENT_WATER constraint to MOLES_BULK_WATER (if there is such a constraint) and
   * all FREE_MOLALITY and FREE_MOLES_MINERAL_SPECIES to MOLES_BULK_SPECIES using the current values
   * of _bulk_moles_old.  If, after performing these changes, all constraints are MOLES_BULK_WATER
   * or MOLES_BULK_SPECIES then charge-balance is performed by altering the bulk_moles_old for the
   * charge-balance species.
   */
  void closeSystem();

  /**
   * Changes the constraint to MOLES_BULK_SPECIES (or MOLES_BULK_WATER if basis_ind = 0) for the
   * basis index.  The constraint value (the bulk number of moles of the species) is computed from
   * the current molality values.  Note that if basis_ind corresponds to a gas, then changing the
   * constraint involves performing a basis swap with a non-gaseous equilibrium species (eg O2(g) is
   * removed from the basis and O2(aq) enters in its place).  If, after performing the change to
   * MOLES_BULK_SPECIES, all constraints are MOLES_BULK_WATER or MOLES_BULK_SPECIES then
   * charge-balance is performed by altering the bulk_moles_old for the charge-balance species.
   * @param basis_ind the index of the basis species
   */
  void changeConstraintToBulk(unsigned basis_ind);

  /**
   * Changes the constraint to MOLES_BULK_SPECIES (or MOLES_BULK_WATER if basis_ind = 0) for the
   * basis index.  The constraint value (the bulk number of moles of the species) is set to value.
   * It is an error to call this function when basis_ind corresponds to a gas, because changing the
   * constraint involves performing a basis swap with a non-gaseous equilibrium species (eg O2(g) is
   * removed from the basis and O2(aq) enters in its place), so the bulk number of moles will be
   * computed by the swapper: use changeConstraintToBulk(basis_ind) instead.    If, after performing
   * the change to MOLES_BULK_SPECIES, all constraints are MOLES_BULK_WATER or MOLES_BULK_SPECIES
   * then charge-balance is performed by altering the bulk_moles_old for the charge-balance species.
   * @param basis_ind the index of the basis species
   * @param value the value to set the bulk number of moles to
   */
  void changeConstraintToBulk(unsigned basis_ind, Real value);

  /**
   * Add to the MOLES_BULK_SPECIES (or MOLES_BULK_WATER if basis_ind = 0) for the basis species.
   * Note that if the constraint on the basis species is not MOLES_BULK_SPECIES (or
   * MOLES_BULK_WATER) then this will have no impact.  Note that charge-balance is performed if all
   * constraints are BULK-type.
   * @param basis_ind the index of the basis species
   * @param value the value to add to the bulk number of moles
   */
  void addToBulkMoles(unsigned basis_ind, Real value);

  /**
   * Set the constraint value for the basis species.  If molalities or activities are changed, this
   * method uses computeConsistentConfiguration to result in a consistent configuration, while if
   * bulk composition is altered it uses alterSystemBecauseBulkChanged to result in a consistent
   * configuration (charge-balance is performed if all constraints are BULK-type)
   * @param basis_ind the index of the basis species
   * @param value the value to set the constraint to
   */
  void setConstraintValue(unsigned basis_ind, Real value);

  /**
   * Compute a reasonably consistent configuration that can be used as an initial condition in a
   * Newton process to solve the algebraic system.  Note that unless _iters_to_make_consistent is
   * very large, the resulting configuration will not be completely consistent, but this is
   * conventional in geochemical modelling: there are so many unknowns and approximations used in
   * the Newton process, that starting from a completely consistent configuration is unimportant.
   *
   * Before entering this method, it is assumed that::
   * - basis molality has been set for all basis species.  For species where this is unknown (viz,
   * it will be solved for in the Newton process) a reasonable initial condition should be set.  Use
   * the initBulkAndFree method.
   * - basis_activity_known has been set to true for basis species whose activities are specified by
   * the user, for basis gases and for basis minerals.  Use the buildKnownBasisActivities method.
   * - basis_activity has been set for basis species with basis_activity_known = true.  Use the
   * buildKnownBasisActivities method.
   * - equilibrium molality has been pre-initialised (eg, to zero) for all equilibrium species prior
   * to entering this method.
   *
   * This method computes:
   * - ionic strength and stoichiometric ionic strength
   * - basis activity coefficients (not for water, minerals or gases)
   * - equilibrium activity coefficients (not for minerals or gases)
   * - basis molality for basis species where the user has specified the activity
   * - basis molality for mineral basis species
   * - basis activity for all basis species
   * - equilibrium molality for all equilibrium species (molality for minerals and gases are set to
   * zero)
   * - bulk_moles_old for all basis species (set to the constraint_value for BULK-type constraints,
   * otherwise computed from the current values of molality)
   * - sorbing surface area
   *
   * It does not compute activity for equilibrium species: use computeAndGetEquilibriumActivity for
   * that
   */
  void computeConsistentConfiguration();

  /**
   * @return the original redox left-hand-side after the initial basis swaps
   */
  const std::string & getOriginalRedoxLHS() const;

  /**
   * Perform the basis swap, and ensure that the resulting system is consistent.  Note: no
   * sanity-checks regarding swapping out the charge-balance species, etc, are made.
   * Note that this
   * alters the bulk_moles_old of species.  If you are confident that your configuration is
   * reasonably correct, you should therefore ensure bulk_moles_old is set to the actual bulk_moles
   * in your system prior to calling performSwap.  Alternatively, if you are unsure of the
   * correctness, just let performSwap use bulk_moles_old to set the bulk moles of your new basis.
   * @param swap_out_of_basis index of basis species to remove from basis
   * @param swap_into_basis index of equilibrium species to add to basis
   */
  void performSwapNoCheck(unsigned swap_out_of_basis, unsigned swap_into_basis);

  /**
   * @return a reference to the swapper used by this object
   */
  const GeochemistrySpeciesSwapper & getSwapper() const;

  /**
   * Set the free mole number of mineral-related species to the value provided.  This sets the mole
   * number of basis minerals, the molality of unoccupied sorption sites (whether in the basis or
   * not), and the molality of sorbed species to the value provided.  It does not set the molality
   * of equilibrium minerals, which is always zero.
   * NOTE: calling this method can easily result in a
   * completely inconsistent GeochemicalSystem because no
   * computeConsistentConfiguration() is called.  If you need a consistent configuration, please
   * call that method after calling this one
   * @param value the mole number (or molality, for sorption-related species)
   */
  void setMineralRelatedFreeMoles(Real value);

  /**
   * Computes activity for all equilibrium species (_eqm_activity) and returns a reference to the
   * vector.  The vector _eqm_activity is only used for output purposes (such as recording the
   * fugacity of equilibrium gases) so it is only computed by this method and not internally during
   * computeConsistentConfiguration.
   * @return equilibrium activities
   */
  const std::vector<Real> & computeAndGetEquilibriumActivity();

  /**
   * Returns the value of activity for the equilibrium species with index eqm_index
   * @param eqm_index the index in mgd for the equilibrium species of interest
   */
  Real getEquilibriumActivity(unsigned eqm_ind) const;

  /**
   * Computes the kinetic rates and their derivatives based on the current values of molality, mole
   * number, etc, and then, using dt, calculates the appropriate mole_additions and dmole_additions.
   * NOTE: this *adds* the kinetic contributes to mole_additions and dmole_additions.
   * This method is not const because it modifies _eqm_activity for
   * equilibrium gases and eqm species H+ and OH- (if there are any).
   * @param dt time-step size
   * @param mole_additions The kinetic rates multiplied by dt get placed in the last num_kin slots
   * @param dmole_additions d(mole_additions)/d(molality or kinetic_moles)
   */
  void
  addKineticRates(Real dt, DenseVector<Real> & mole_additions, DenseMatrix<Real> & dmole_additions);

private:
  /// The minimal geochemical database
  ModelGeochemicalDatabase & _mgd;
  /// number of basis species
  const unsigned _num_basis;
  /// number of equilibrium species
  const unsigned _num_eqm;
  /// number of redox species
  const unsigned _num_redox;
  /// number of surface potentials
  const unsigned _num_surface_pot;
  /// number of kinetic species
  const unsigned _num_kin;
  /// swapper that can swap species into and out of the basis
  GeochemistrySpeciesSwapper & _swapper;
  /// Species to immediately remove from the basis in favor of _swap_in
  const std::vector<std::string> _swap_out;
  /// Species to immediately add to the basis in favor of _swap_out
  const std::vector<std::string> _swap_in;
  /// Object to compute the activity coefficients and activity of water
  GeochemistryActivityCoefficients & _gac;
  /// Object that provides the ionic strengths
  GeochemistryIonicStrength & _is;
  /// The species used to enforce charge balance
  std::string _charge_balance_species;
  /// The species used to enforce charge balance, as provided in the constructor
  const std::string _original_charge_balance_species;
  /// The index in the list of basis species corresponding to the charge-balance species.  This gets altered as the simulation progresses, due to swaps, and alterChargeBalanceSpecies
  unsigned _charge_balance_basis_index;
  /// Names of the species in that have their values fixed to _constraint_value with meanings _constraint_meaning.  In the constructor, this is ordered to have the same ordering as the basis species.
  std::vector<std::string> _constrained_species;
  /// Numerical values of the constraints on _constraint_species.  In the constructor, this is ordered to have the same ordering as the basis species.
  std::vector<Real> _constraint_value;
  /// Numerical values of the constraints on _constraint_species.  In the constructor, this is ordered to have the same ordering as the basis species.  Since values can change due to charge-balance, this holds the original values set by the user.
  std::vector<Real> _original_constraint_value;
  /// Units of the _constraint_value when the GeochemicalSystem is constructed.  This is used during the constructor to convert the constraint_values into mole units
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> _constraint_unit;
  /// The user-defined meaning of the values in _constraint_value.  In the constructor, this is ordered to have the same ordering as the basis species.  During the process of unit conversion, from the user-supplied _constraint_unit, to mole-based units, _constraint_user_meaning is used to populate _constraint_meaning, and henceforth usually only _constraint_meaning is used in the code
  std::vector<ConstraintUserMeaningEnum> _constraint_user_meaning;
  /// The meaning of the values in _constraint_value.  In the constructor, this is ordered to have the same ordering as the basis species.
  std::vector<ConstraintMeaningEnum> _constraint_meaning;
  /// equilibrium constant of the equilibrium species
  std::vector<Real> _eqm_log10K;
  /// equilibrium constant of the redox species
  std::vector<Real> _redox_log10K;
  /// equilibrium constant of the kinetic species
  std::vector<Real> _kin_log10K;
  /// number of unknown molalities in the algebraic system.  Note: surface potentials (if any) are extra quantities in the algebraic system
  unsigned _num_basis_in_algebraic_system;
  /// number of unknowns in the algebraic system (includes molalities and surface potentials)
  unsigned _num_in_algebraic_system;
  /// if _in_algebraic_system[i] == true then we need to solve for the i^th basis-species molality
  std::vector<bool> _in_algebraic_system;
  /// _algebraic_index[i] = index in the algebraic system of the basis species i.  _basis_index[_algebraic_index[i]] = i
  std::vector<unsigned> _algebraic_index;
  /// _basis_index[i] = index in the basis of the algebraic system species i, for i<num_basis_in_algebraic_system.  _basis_index[_algebraic_index[i]] == i
  std::vector<unsigned> _basis_index;
  /// Number of bulk moles of basis species (this includes contributions from kinetic species, in contrast to Bethke)
  std::vector<Real> _bulk_moles_old;
  /**
   * IMPORTANT: this is
   * - number of kg of solvent water as the first element
   * - molality of basis aqueous species, or free moles of basis mineral, whichever is appropriate
   * - always zero for gases
   */
  std::vector<Real> _basis_molality;
  /// whether basis_activity[i] is fixed by the user
  std::vector<bool> _basis_activity_known;
  /// values of activity (for water, minerals and aqueous basis species) or fugacity (for gases)
  std::vector<Real> _basis_activity;
  /// molality of equilibrium species
  std::vector<Real> _eqm_molality;
  /// basis activity coefficients
  std::vector<Real> _basis_activity_coef;
  /// equilibrium activity coefficients
  std::vector<Real> _eqm_activity_coef;
  /**
   * equilibrium activities.  NOTE: for computational efficiency, these are not computed until
   * computeAndGetEqmActivity is called, and they are currently only ever used for output purposes
   * and computing kinetic rates
   */
  std::vector<Real> _eqm_activity;
  /**
   * surface potential expressions.  These are not the surface potentials themselves.  Instead they
   * are exp(-surface_potential * Faraday / 2 / R / T_k).  Hence _surface_pot_expr >= 0 (like
   * molalities) and the surface-potential residual is close to linear in _surface_pot_expr if
   * equilibrium molalities are large
   */
  std::vector<Real> _surface_pot_expr;
  /// surface areas of the sorbing minerals
  std::vector<Real> _sorbing_surface_area;
  /// mole number of kinetic species
  std::vector<Real> _kin_moles;
  /// old mole number of kinetic species
  std::vector<Real> _kin_moles_old;
  /**
   * Iterations to make the initial configuration consistent.  Note that because equilibrium
   * molality depends on activity and activity coefficients, and water activity and activity
   * coefficients depend on molality, it is a nontrivial task to compute all these so that the
   * algorithm starts with a consistent initial condition from which to solve the algebraic system.
   * Usually the algorithm doesn't even attempt to make a consistent initial condition (a suitable
   * default is iters_to_make_consistent=0), because solving the algebraic system includes so many
   * approximations anyway.
   */
  unsigned _iters_to_make_consistent;
  /// The temperature in degC
  Real _temperature;
  /// Minimum molality ever used in an initial guess
  Real _min_initial_molality;
  /// The left-hand-side of the redox equations in _mgd after initial swaps
  std::string _original_redox_lhs;

  /**
   * Using the provided value of temperature, build _eqm_log10K for each eqm species and redox
   * species
   */
  void buildTemperatureDependentQuantities(Real temperature);

  /// Builds in_algebraic_system, algebraic_index and basis_index, and sets num_basis_in_algebraic_system appropriately
  void buildAlgebraicInfo(std::vector<bool> & in_algebraic_system,
                          unsigned & num_basis_in_algebraic_system,
                          unsigned & num_in_algebraic_system,
                          std::vector<unsigned> & algebraic_index,
                          std::vector<unsigned> & basis_index) const;

  /**
   * based on _constrained_value and _constraint_meaning, populate nw, bulk_moles_old and
   * basis_molality with reasonable initial conditions that may be used during the Newton solve of
   * the algebraic system
   * @param bulk_moles_old bulk composition number of moles of the basis species
   * @param basis_molality zeroth component is mass of solvent water, other components are either
   * molality of the basis aqueous species or number of moles of basis mineral, whichever is
   * appropriate
   */
  void initBulkAndFree(std::vector<Real> & bulk_moles_old,
                       std::vector<Real> & basis_molality) const;

  /**
   * Fully populates basis_activity_known, which is true if activity has been set by the user, or
   * the fugacity has been set by the user, or the basis species is a mineral. Populates only those
   * slots in basis_activity for which basis_activity_known == true
   */
  void buildKnownBasisActivities(std::vector<bool> & basis_activity_known,
                                 std::vector<Real> & basis_activity) const;

  /**
   * Compute the activity of water and put in basis_activity[0], and use the _basis_activity_coef
   * and _basis_molality to compute the remaining basis activities (for those that are not minerals,
   * gases or aqueous species provided with an activity)
   */
  void computeRemainingBasisActivities(std::vector<Real> & basis_activity) const;

  /**
   * For basis aqueous species (not water, minerals or gases) with activity set by the user, set
   * basis_molality = activity / activity_coefficient
   */
  void updateBasisMolalityForKnownActivity(std::vector<Real> & basis_molality) const;

  /**
   * @return log10(activity product) for equilibrium species eqm_j
   */
  Real log10ActivityProduct(unsigned eqm_j) const;

  /**
   * compute the equilibrium molalities (not for minerals or gases)
   */
  void computeEqmMolalities(std::vector<Real> & eqm_molality) const;

  /**
   * If all non-charged basis species are provided with a bulk number of moles, alter the bulk
   * number of moles (old) of the charge_balance_species so that charges are balanced
   * @param constraint_value the values of the constraints provided by the user
   * @param bulk_moles_old the old values of bulk moles (from previous time-step)
   */
  void enforceChargeBalanceIfSimple(std::vector<Real> & constraint_value,
                                    std::vector<Real> & bulk_moles_old) const;

  /**
   * Computes the value of bulk_moles_old for all basis species.  For species with BULK-type
   * constraints bulk_moles_old gets set to the constraint value, while for other species
   * bulk_moles_old is computed from the molalities, and, in contrast to Bethke, the kinetic species
   */
  void computeBulk(std::vector<Real> & bulk_moles_old) const;

  /**
   * Enforces charge balance by altering the constraint_value and bulk_moles_old of the
   * charge-balance species.  Note that this changes the important constraint_value and the old
   * bulk_moles_old (usually from the previous time-step) so should be used with caution.
   */
  void enforceChargeBalance(std::vector<Real> & constraint_value,
                            std::vector<Real> & bulk_moles_old) const;

  /**
   * Compute the free mineral moles (ie, basis_molality for basis species that are minerals), using
   * the bulk_moles_old.  Note that usually computeBulk should have been called immediately
   * preceding this in order to correctly set bulk_moles_old.
   */
  void computeFreeMineralMoles(std::vector<Real> & basis_molality) const;

  /**
   * Used during construction: checks for sane inputs and initializes molalities, etc, using the
   * initialize() method
   * @param kin_name names of kinetic species
   * @param kin_initial initial mole numbers (or mass or volume, depending on kin_unit) of the
   * species named in kin_name
   * @param kin_unit units of the numbers provided in kin_initial
   */
  void
  checkAndInitialize(const std::vector<std::string> & kin_name,
                     const std::vector<Real> & kin_initial,
                     const std::vector<GeochemistryUnitConverter::GeochemistryUnit> & kin_unit);

  /**
   * Set the charge-balance species to the basis index provided.  No checks are made on the
   * sanity of the desired change. Before setting, the _constraint_value mole number of the old
   * charge-balance species is set to the value provided in the constructor.  Then
   * _charge_balance_basis_index and _charge_balance_species is set appropriately
   */
  void setChargeBalanceSpecies(unsigned new_charge_balance_index);

  /**
   * @param eqm_j the index of the equilibrium species
   * @return the modifier to the mass-balance equation for equilibrium species j, which is
   * exp(-z * psi * F / R / TK), where z is the charge of the equilibrium species j, psi is the
   * surface potential relevant to the equilibrium species j, F the Faraday constant, R the gas
   * constant, and TK the temperature in Kelvin.  If equilibrium species j has no relevant surface
   * potential, unity is returned.  Note that exp(-z * psi * F / R / TK) = (_surface_pot_expr)^(2z)
   */
  Real surfaceSorptionModifier(unsigned eqm_j) const;

  /**
   * @param sp Surface potential number.  sp < _num_surface_pot
   * @return (1/2) * A / F * sqrt(R * T_k * eps * eps_0 * rho * I).  This appears in the
   * surface-potential equation.
   */
  Real surfacePotPrefactor(unsigned sp) const;

  /// Compute sorbing_surface_area depending on the current molality of the sorbing minerals
  void computeSorbingSurfaceArea(std::vector<Real> & sorbing_surface_area) const;

  /**
   * @param basis_ind the index of the basis species in mgd
   * @return the number of bulk moles of the species basis_ind.  This depends on the current
   * molality of the basis_ind species, as well as the current molalities of the equilibrium species
   * that depend on this basis species, and, in contrast to the Bethke approach, the current mole
   * number of kinetic species that depend on this basis species.
   */
  Real computeBulkFromMolalities(unsigned basis_ind) const;

  /**
   * Alter the GeochemicalSystem to reflect changes in bulk composition constraints that
   * occur through, for instance, setConstraintValue or addToBulkMoles or changeConstraintToBulk
   * (the latter because charge neutrality might be easily enforced, changing constraints and hence
   * bulk moles). This method enforces charge neutrality if simple (ie, all constraints are
   * BULK-type), then sets _bulk_moles_old, and free mineral moles appropriately
   */
  void alterSystemBecauseBulkChanged();
};
