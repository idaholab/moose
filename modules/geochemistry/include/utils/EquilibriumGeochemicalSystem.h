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

/**
 * This class holds information about bulk composition, molalities, activities, activity
 * coefficients, etc of the user-defined geochemical system in PertinentGeochemicalSystem.  Hence,
 * it extends the generic information in PertinentGeochemicalSystem to a system that is specific to
 * one spatio-temporal location.  It offers methods to manipulate these molalities, bulk
 * compositions, etc, as well as performing basis swaps.
 *
 * Related to this, this class also builds the so-called "algebraic system" which is the nonlinear
 * system of algebraic equations for the unknown basis molalities and surface potentials, and
 * computes the residual vector and jacobian of this algebraic system.  This class initialises the
 * algebraic variables (the unknown molalities and surface potentials) to reaponable values, but it
 * does not solve the algebraic system: instead, it has a "setter" method, setAlgebraicValues that
 * can be used to set the unknowns, hence another class can use whatever method it desires to solve
 * the system.
 *
 * WARNING: because EquilibriumGeochemicalSystem performs swaps, it will change the
 * ModelGeochemicalDatabase object.
 * WARNING: because EquilibriumGeocehmicalSystem sets internal parameters in the
 * activity-coefficient object, it will change the GeochemistryActivityCoefficient object.
 */
class EquilibriumGeochemicalSystem
{
public:
  /// Each basis species must be provided with a constraint, that is chosen from the following enum
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

  /**
   * Construct the equilibrium geochemical system, check consistency of the constructor's arguments,
  and initialize all internal variables (molalities, bulk compositions, equilibrium constants,
  activities and activity coefficients) and set up the algebraic system
   * @param mgd the model's geochemical database, which is a model-specific version of the full
  geochemical database.  WARNING: because EquilibriumGeochemicalSystem performs swaps, it will
  change mgd
  * @param gac the object that computes activity coefficients.  WARNING: because
  EquilibriumGeochemicalSystem sets internal parameters in gac, it will change it.
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
   * @param constraint_meaning A list the provides physical meaning to the constraint_values
   * @param initial_temperature Initial temperature
   * @param iters_to_make_consistent The initial equilibrium molalities depend on the activity
  coefficients, which depend on the basis and equilibrium molalities.  This circular dependence
  means it is usually impossible to define an exactly consistent initial configuration for
  molalities.  An iterative process is used to approach the consistent initial configuration, using
  iters_to_make_consistent iterations.  Usually iters_to_make_consistent=0 is reasonable, because
  there are so many approximations used in the solution process that a slightly inconsistent initial
  configuration is fine
   * @param min_initial_molality Minimum value of equilibrium molality used in the initial condition
   */
  EquilibriumGeochemicalSystem(ModelGeochemicalDatabase & mgd,
                               GeochemistryActivityCoefficients & gac,
                               const GeochemistryIonicStrength & is,
                               GeochemistrySpeciesSwapper & swapper,
                               const std::vector<std::string> & swap_out_of_basis,
                               const std::vector<std::string> & swap_into_basis,
                               const std::string & charge_balance_species,
                               const std::vector<std::string> & constrained_species,
                               const std::vector<Real> & constraint_value,
                               const MultiMooseEnum & constraint_meaning,
                               Real initial_temperature,
                               unsigned iters_to_make_consistent,
                               Real min_initial_molality);

  EquilibriumGeochemicalSystem(ModelGeochemicalDatabase & mgd,
                               GeochemistryActivityCoefficients & gac,
                               const GeochemistryIonicStrength & is,
                               GeochemistrySpeciesSwapper & swapper,
                               const std::vector<std::string> & swap_out_of_basis,
                               const std::vector<std::string> & swap_into_basis,
                               const std::string & charge_balance_species,
                               const std::vector<std::string> & constrained_species,
                               const std::vector<Real> & constraint_value,
                               const std::vector<ConstraintMeaningEnum> & cc,
                               Real initial_temperature,
                               unsigned iters_to_make_consistent,
                               Real min_initial_molality);

  /// returns the number of species in the basis
  unsigned getNumInBasis() const;

  /// returns the number of species in equilibrium with the basis components
  unsigned getNumInEquilibrium() const;

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
   * @param red the redox species in disequilibrium
   * @return the value of log10(equilibrium constant) for the given disequilibrium-redox species
   */
  Real getRedoxLog10K(unsigned red) const;

  /**
   * @param red the redox species in disequilibrium
   * @return the value of log10(activity product) for the given disequilibrium-redox species
   */
  Real log10RedoxActivityProduct(unsigned red) const;

  /// return the number in the algebraic system (number of basis species in algebraic system + number of surface potentials)
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
   * current values of the algebraic variables: molalities only (not surface potentials)
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
   * @return the number of bulk-composition moles
   */
  const std::vector<Real> & getBulkMoles() const;

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

  /// Returns the total charge in the system
  Real getTotalCharge() const;

  /**
   * Return the residual of the algebraic system for the given algebraic index
   * @param algebraic_ind the algebraic index
   */
  Real getResidualComponent(unsigned algebraic_ind) const;

  /**
   * @return reference to the underlying ModelGeochemicalDatabase
   */
  const ModelGeochemicalDatabase & getModelGeochemicalDatabase() const;

  /**
   * Compute the Jacobian for the algebraic system and put it in jac
   * @param res The residual of the algebraic system.  This is used to speed up computations of the
   * jacobian
   */
  void computeJacobian(const DenseVector<Real> & res, DenseMatrix<Real> & jac) const;

  /**
   * Set the variables in the algebraic system (molalities and potentially the mass of solvent
   * water, and surface potentials, if any) to algebraic_var.  The first
   * _num_basis_in_algebraic_system elements of this vector (corresponding to basis molalities and
   * mass of solvent water) must be positive. This function also does a lot more:
   * - recomputes new ionic strengths
   * - builds all activity coefficients for basis species that have not got fixed activity
   * - builds all activity coefficients for equilibium aqueous species (not minerals or gases)
   * - updates basis molality for species with fixed activity
   * - recomputes all basis activities
   * - recomputes the molality for equilibrium aqueous species (not minerals or gases)
   * - recomputes the bulk composition for all basis species
   * Because of these computations, the EquilibriumGeochemicalSystem is kept in an
   * internally-consistent state.
   * @param algebraic_var the values to set the algebraic variables to
   */
  void setAlgebraicVariables(const DenseVector<Real> & algebraic_var);

  /**
   * Enforces charge balance by altering the constraint_value and bulk_moles of the charge-balance
   * species.  Use with caution, since this overwrites the constraint values provided in the
   * constructor
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
   * Perform the basis swap, and ensure that the resulting system is consistent
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
  /// swapper that can swap species into and out of the basis
  GeochemistrySpeciesSwapper & _swapper;
  /// Species to immediately remove from the basis in favor of _swap_in
  const std::vector<std::string> _swap_out;
  /// Species to immediately add to the basis in favor of _swap_out
  const std::vector<std::string> _swap_in;
  /// Object to compute the activity coefficients and activity of water
  GeochemistryActivityCoefficients & _gac;
  /// Object that provides the ionic strengths
  const GeochemistryIonicStrength & _is;
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
  /// The meaning of the values in _constraint_value.  In the constructor, this is ordered to have the same ordering as the basis species.
  std::vector<ConstraintMeaningEnum> _constraint_meaning;
  /// equilibrium constant of the equilibrium species
  std::vector<Real> _eqm_log10K;
  /// equuilibrium constant of the redox species
  std::vector<Real> _redox_log10K;
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
  /// Number of bulk moles of basis species
  std::vector<Real> _bulk_moles;
  /// Number of kg of solvent water, molality of basis aqueous species, or free moles of basis mineral, whichever is appropriate
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
   * surface potential expressions.  These are not the surface potentials themselves.  Instead they
   * are exp(-surface_potential * Faraday / 2 / R / T_k).  Hence _surface_pot_expr >= 0 (like
   * molalities) and the surface-potential residual is close to linear in _surface_pot_expr if
   * equilibrium molalities are large
   */
  std::vector<Real> _surface_pot_expr;
  /// surface areas of the sorbing minerals
  std::vector<Real> _sorbing_surface_area;
  /**
   * Iterations to make the initial configuration consistent.  Note that because equilibrium
   * molality depends on activity and activity coefficients, and water activity and activity
   * coefficients depend on molality, it is a nontrivial task to compute all these so that the
   * algorithm starts with a consistent initial condition from which to solve the algebraic system.
   * Usually the algorithm doesn't even attempt to make a consistent initial condition (a suitable
   * default is iters_to_make_consistent=0), because solving the algebraic system includes so many
   * approximations anyway.
   */
  const unsigned _iters_to_make_consistent;
  /// The temperature
  Real _temperature;
  /// Minimum molality ever used in an initial guess
  const Real _min_initial_molality;

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
   * based on _constrained_value and _constrained_meaning, populate nw, bulk_moles and
   * basis_molality with reasonable initial conditions that may be used during the Newton solve of
   * the algebraic system
   * @param bulk_moles bulk composition number of moles of the basis species
   * @param basis_molality zeroth component is mass of solvent water, other components are either
   * molality of the basis aqueous species or number of moles of basis mineral, whichever is
   * appropriate
   */
  void initBulkAndFree(std::vector<Real> & bulk_moles, std::vector<Real> & basis_molality) const;

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
   * number of moles of the charge_balance_species so that charges are balanced
   * @param constraint_value the values of the constraints provided by the user
   */
  void enforceChargeBalanceIfSimple(std::vector<Real> & constraint_value) const;

  /**
   * Computes the value of _bulk_moles for all basis species
   */
  void computeBulk(std::vector<Real> & bulk_moles) const;

  /**
   * Enforces charge balance by altering the constraint_value and bulk_moles of the charge-balance
   * species
   */
  void enforceChargeBalance(std::vector<Real> & constraint_value,
                            std::vector<Real> & bulk_moles) const;

  /**
   * Compute the free mineral moles (ie, basis_molality for basis species that are minerals), using
   * the bulk mineral moles.  Note that usually computeBulk should have been called immediately
   * preceding this
   */
  void computeFreeMineralMoles(std::vector<Real> & basis_molality) const;

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
   * - equilibrium molality for all equilibrium species except minerals and gases
   * - bulk mole number for all basis species
   */
  void computeConsistentConfiguration();

  /// Used during construction: checks for sane inputs and initializes molalities, etc, using the initialize() method
  void checkAndInitialize();

  /**
   * Set the charge-balance species to the basis index provided.  No checks are made on the sanity
   * of the desired change. Before setting, the _constraint_value mole number of the old
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
};
