//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include "DelimitedFileReader.h"

class CrystalPlasticityStressUpdateBase : public Material
{
public:
  static InputParameters validParams();

  CrystalPlasticityStressUpdateBase(const InputParameters & parameters);

  /// Sets the value of the global variable _qp for inheriting classes
  void setQp(const unsigned int & qp);

  /// Sets the value of the _substep_dt for inheriting classes
  void setSubstepDt(const Real & substep_dt);

  ///@{ Retained as empty methods to avoid a warning from Material.C in framework. These methods are unused in all inheriting classes and should not be overwritten.
  virtual void resetQpProperties() final {}
  virtual void resetProperties() final {}
  ///@}

  /**
   * initializes the stateful properties such as PK2 stress, resolved shear
   * stress, plastic deformation gradient, slip system resistances, etc.
   * This class is often overwritten by inherting classes.
   */
  virtual void initQpStatefulProperties() override;

  /**
   * A helper method to read in plane normal and direction vectors from a file
   * and to normalize the vectors. This method is abstracted to allow for reuse
   * in inheriting classes with multiple plane normal and direction vector pairs.
   */
  virtual void getSlipSystems();

  /**
   * A helper method to transform the Miller-Bravais 4-index notation for HCP
   * crystals into a a 3-index Cartesian representation, using the convention
   * a$_1$ = x of axis alignment in the basal plane
   */
  void transformHexagonalMillerBravaisSlipSystems(const MooseUtils::DelimitedFileReader & reader);

  /**
   * Computes the Schmid tensor (m x n) for the original (reference) crystal
   * lattice orientation for each glide slip system
   */
  void calculateFlowDirection(const RankTwoTensor & crysrot);

  /**
   * Computes the shear stess for each slip system
   */
  void calculateShearStress(const RankTwoTensor & pk2,
                            const RankTwoTensor & inverse_eigenstrain_deformation_grad,
                            const unsigned int & num_eigenstrains);

  /**
   * Calculates the total value of $\frac{d\mathbf{F}^P^{-1}}{d\mathbf{PK2}}$ and
   * is intended to be an overwritten helper method for inheriting classes with
   * multiple constitutive dislocation slip mechanisms, e.g. glide and twinning,
   * $\sum_i \frac{d\mathbf{F}^P^{-1}}{d\mathbf{PK2}_i}$
   */
  virtual void calculateTotalPlasticDeformationGradientDerivative(
      RankFourTensor & dfpinvdpk2,
      const RankTwoTensor & inverse_plastic_deformation_grad_old,
      const RankTwoTensor & inverse_eigenstrain_deformation_grad_old,
      const unsigned int & num_eigenstrains);

  /**
   * A helper method to rotate the a direction and plane normal system set into
   * the local crystal llatice orientation as defined by the crystal rotation
   * tensor from the Elasticity tensor class.
   */
  void calculateSchmidTensor(const unsigned int & number_dislocation_systems,
                             const std::vector<RealVectorValue> & plane_normal_vector,
                             const std::vector<RealVectorValue> & direction_vector,
                             std::vector<RankTwoTensor> & schmid_tensor,
                             const RankTwoTensor & crysrot);

  /**
   * A helper method to sort the slip systems of a crystal into cross slip families based
   * on common slip directions.  This method determines if slip directions are parallel,
   * and stores the index of the slip systems from getSlipSystems (the same index is used
   * for the applied shear stress and the internal state variables) in a vector of vectors,
   * where the outer vector separates the individual slip system families and the inner vector
   * stories the indices of the slip systems within a single family.  This vector of vectors
   * can then be used in the inheriting classes to calculate cross slip interactions.
   * The values of number_cross_slip_directions and number_cross_slip_planes must be set to
   * use this sorting method.
   */
  void sortCrossSlipFamilies();

  /**
   * A helper method for inherting classes to identify to which cross slip family vector
   * a particular slip system index belongs after the slip systems have been sorted. Returns
   * the integer value of the identified cross slip system family for the outer vector
   * created in sortCrossSlipFamilies.
   */
  unsigned int identifyCrossSlipFamily(const unsigned int index);

  /**
   * This virtual method is called to set the constitutive internal state variables
   * current value and the previous substep value to the old property value for
   * the start of the stress convergence while loop.
   */
  virtual void setInitialConstitutiveVariableValues() {}

  /**
   * This virtual method is called to set the current constitutive internal state
   * variable value to that of the previous substep at the beginning of the next
   * substep increment. In cases where only one substep is taken (or when the first)
   * substep is taken, this method sets the current constitutive internal state
   * variable value to the old value.
   */
  virtual void setSubstepConstitutiveVariableValues() {}

  /**
   * Stores the current value of the constitutive internal state variables into
   * a separate material property in case substepping is required, once the
   * constitutive variables have passed convergence tolerances. This separate
   * material property is used as the previous substep value in the associated
   * setSubstepConstitutiveVariableValues method in the next substep (if taken).
   */
  virtual void updateSubstepConstitutiveVariableValues() {}

  /**
   * This virtual method is called to calculate the slip system slip
   * increment based on the constitutive model defined in the child class.
   * This method must be overwritten in the child class.
   */
  virtual bool calculateSlipRate() = 0;

  virtual void calculateEquivalentSlipIncrement(RankTwoTensor & /*equivalent_slip_increment*/);

  /**
   * This virtual method is called to find the derivative of the slip increment
   * with respect to the applied shear stress on the slip system based on the
   * constiutive model defined in the child class.  This method must be overwritten
   * in the child class.
   */
  virtual void calculateConstitutiveSlipDerivative(std::vector<Real> & /*dslip_dtau*/) = 0;

  /**
   * Finalizes the values of the state variables and slip system resistance
   * for the current timestep after convergence has been reached.
   */

  virtual void cacheStateVariablesBeforeUpdate() {}

  virtual void calculateStateVariableEvolutionRateComponent() {}

  /**
   * Finalizes the values of the state variables and slip system resistance
   * for the current timestep after convergence has been reached.
   */
  virtual bool updateStateVariables() = 0;

  virtual void calculateSlipResistance() {}

  /**
   * Determines if all the state variables have converged
   */
  virtual bool areConstitutiveStateVariablesConverged() { return true; }

  /**
   * Check if a typical state variable, e.g. defect density, has converged
   * by comparing the change in the values over the iteration period.
   */
  virtual bool isConstitutiveStateVariableConverged(const std::vector<Real> & current_var,
                                                    const std::vector<Real> & var_before_update,
                                                    const std::vector<Real> & previous_substep_var,
                                                    const Real & tolerance);

protected:
  /// Base name prepended to all material property names to allow for
  /// multi-material systems
  const std::string _base_name;

  const enum class CrystalLatticeType { BCC, FCC, HCP } _crystal_lattice_type;

  const std::vector<Real> _unit_cell_dimension;

  ///Maximum number of active slip systems for the crystalline material being modeled
  const unsigned int _number_slip_systems;

  /// File should contain slip plane normal and direction.
  std::string _slip_sys_file_name;

  /// @{Parameters to characterize the cross slip behavior of the crystal
  const Real _number_cross_slip_directions;
  const Real _number_cross_slip_planes;
  ///@}

  /// Internal variable update equation tolerance
  Real _rel_state_var_tol;
  /// Slip increment tolerance
  Real _slip_incr_tol;
  /// Tolerance for change in slip system resistance over an increment
  Real _resistance_tol;
  /// Residual tolerance when variable value is zero. Default 1e-12.
  Real _zero_tol;

  ///@{Slip system resistance
  MaterialProperty<std::vector<Real>> & _slip_resistance;
  const MaterialProperty<std::vector<Real>> & _slip_resistance_old;
  ///@}

  /// Current slip increment material property
  MaterialProperty<std::vector<Real>> & _slip_increment;

  ///@{Slip system direction and normal and associated Schmid tensors
  std::vector<RealVectorValue> _slip_direction;
  std::vector<RealVectorValue> _slip_plane_normal;
  MaterialProperty<std::vector<RankTwoTensor>> & _flow_direction;
  ///@}

  /// Resolved shear stress on each slip system
  MaterialProperty<std::vector<Real>> & _tau;

  /// Flag to print to console warning messages on stress, constitutive model convergence
  const bool _print_convergence_message;

  /// Substepping time step value used within the inheriting constitutive models
  Real _substep_dt;

  /// Sorted slip system indices into cross slip family groups
  std::vector<std::vector<unsigned int>> _cross_slip_familes;

  /// Flag to run the cross slip calculations if cross slip numbers are specified
  bool _calculate_cross_slip;
};
