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

/**
 * CrystalPlasticityUpdate uses the multiplicative decomposition of the
 * deformation gradient and solves the PK2 stress residual equation at the
 * intermediate configuration to evolve the material state. The internal
 * variables are updated using an interative predictor-corrector algorithm.
 * Backward Euler integration rule is used for the rate equations.
 *
 * This material that is not called by MOOSE because of the compute=false flag
 * set in the parameter list. All materials inheriting from this class must be
 * called by a separate material, such as ComputeCrystalPlasticityStress.
 */
class CrystalPlasticityUpdate : public Material
{
public:
  static InputParameters validParams();

  CrystalPlasticityUpdate(const InputParameters & parameters);

  /**
   * Updates the stress (PK2) at a quadrature point by calling constiutive
   * relationship as defined in a child class
   * Solves stress residual equation using Newton - Rhapson: Updates slip
   * system resistances iteratively
   */
  virtual void updateStress(RankTwoTensor & cauchy_stress, RankFourTensor & jacobian_mult);

  /// Sets the value of the global variable _qp for inheriting classes
  void setQp(unsigned int qp);

  ///@{ Retained as empty methods to avoid a warning from Material.C in framework. These methods are unused in all inheriting classes and should not be overwritten.
  virtual void resetQpProperties() final {}
  virtual void resetProperties() final {}
  ///@}

protected:
  /**
   * initializes the stateful properties such as PK2 stress, resolved shear
   * stress, plastic deformation gradient, slip system resistances, etc.
   * This class is often overwritten by inherting classes.
   */
  virtual void initQpStatefulProperties() override;

  /**
   * Calls the residual and jacobian functions used in the stress update
   * algorithm.
   */
  void calculateResidualAndJacobian();

  /**
   * Reset the PK2 stress and the inverse deformation gradient to old values and
   * provide an interface for inheriting classes to reset material properties
   */
  void preSolveQp();

  /**
   * Solve the stress and internal state variables (e.g. slip increment,
   * slip system resistance) at each qp points
   */
  void solveQp();

  /**
   * Save the final stress and internal variable values after the iterative solve.
   */
  void postSolveQp(RankTwoTensor & stress_new, RankFourTensor & jacobian_mult);

  /**
   * Solves the internal variables stress as a function of the slip specified
   * by the constitutive model defined in the inheriting class
   */
  void solveStateVariables();

  /**
   * solves for stress, updates plastic deformation gradient.
   */
  void solveStress();

  /**
   * Calculate stress residual as the difference between the stored material
   * property PK2 stress and the elastic PK2 stress calculated from the
   * constitutively defined equivalent_slip_increment.
   * The equivalent_slip_increment is passed in as an input arguement.
   */
  void calcResidual();

  /**
   * Calculates the jacobian as
   * $\mathbf{J} = \mathbf{I} - \mathbf{C} \frac{d\mathbf{E}^e}{d\mathbf{F}^e}
   * \frac{d\mathbf{F}^e}{d\mathbf{F}^P^{-1}} \frac{d\mathbf{F}^P^{-1}}{d\mathbf{PK2}}$
   */
  void calcJacobian();

  /**
   * Calculates the total value of $\frac{d\mathbf{F}^P^{-1}}{d\mathbf{PK2}}$ and
   * is intended to be an overwritten helper method for inheriting classes with
   * multiple constitutive dislocation slip mechanisms, e.g. glide and twinning,
   * $\sum_i \frac{d\mathbf{F}^P^{-1}}{d\mathbf{PK2}_i}$
   * In this base class implementation with only dislocation glide assumed, no
   * specific sum is necessary.
   */
  virtual void calculateTotalPlasticDeformationGradientDerivative(RankFourTensor & dfpinvdpk2);

  /**
   * Calculates the value of $\frac{d\mathbf{F}^P^{-1}}{d\mathbf{PK2}}$ for a specific
   * constitutive model based on the values of the Schmid tensor and number of
   * dislocation movement systems (e.g. glide slip systems) provided by the calling
   * class. Note that this class is written in a general manner to allow for reuse
   * by the calling calculateTotalPlasticDeformationGradientDerivative class for
   * a variable number of constitutive dislocation slip mechanisms.
   */
  void calculateConstitutivePlasticDeformationGradientDerivative(
      RankFourTensor & dfpinvdpk2,
      std::vector<RankTwoTensor> & schmid_tensor,
      unsigned int slip_model_number = 0);

  ///@{Calculates the tangent moduli for use as a preconditioner, using the elastic or elastic-plastic option as specified by the user
  void calcTangentModuli(RankFourTensor & jacobian_mult);
  void elasticTangentModuli(RankFourTensor & jacobian_mult);
  void elastoPlasticTangentModuli(RankFourTensor & jacobian_mult);
  ///@}

  /// performs the line search update
  bool lineSearchUpdate(const Real & rnorm_prev, const RankTwoTensor & dpk2);

  /**
   * Computes the Schmid tensor (m x n) for the original (reference) crystal
   * lattice orientation for each glide slip system
   */
  void calculateFlowDirection();

  /**
   * A helper method to rotate the a direction and plane normal system set into
   * the local crystal llatice orientation as defined by the crystal rotation
   * tensor from the Elasticity tensor class.
   */
  void calculateSchmidTensor(const unsigned int & number_dislocation_systems,
                             const DenseVector<Real> & plane_normal_vector,
                             const DenseVector<Real> & direction_vector,
                             std::vector<RankTwoTensor> & schmid_tensor);

  /// Read in the crystal specific glide slip systems from a file
  void getSlipSystems();

  /**
   * A helper method to read in plane normal and direction vectors from a file
   * and to normalize the vectors. This method is abstracted to allow for reuse
   * in inheriting classes with multiple plane normal and direction vector pairs.
   */
  void getPlaneNormalAndDirectionVectors(const FileName & vector_file_name,
                                         const unsigned int & number_dislocation_systems,
                                         DenseVector<Real> & plane_normal_vector,
                                         DenseVector<Real> & direction_vector,
                                         bool & orthonormal_error);

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
  unsigned int indentifyCrossSlipFamily(const unsigned int index);

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
   * This virtual method is called to calculate the total slip system slip
   * increment based on the constitutive model defined in the child class.
   * This method must be overwritten in the child class.
   */
  virtual void
  calculateConstitutiveEquivalentSlipIncrement(RankTwoTensor & /*equivalent_slip_increment*/,
                                               bool & /*error_tolerance*/)
  {
  }

  /*
   * This virtual method is called to find the derivative of the slip increment
   * with respect to the applied shear stress on the slip system based on the
   * constiutive model defined in the child class.  This method must be overwritten
   * in the child class.
   */
  virtual void calculateConstitutiveSlipDerivative(std::vector<Real> & /*dslip_dtau*/,
                                                   unsigned int /*slip_model_number*/ = 0)
  {
  }

  /*
   * Finalizes the values of the state variables and slip system resistance
   * for the current timestep after convergence has been reached.
   */
  virtual void updateConstitutiveSlipSystemResistanceAndVariables(bool & /*error_tolerance*/) {}

  /*
   * Determines if the state variables, e.g. defect densities, have converged
   * by comparing the change in the values over the iteration period.
   */
  virtual bool areConstitutiveStateVariablesConverged() { return true; }

  /// optional parameter to define several mechanical systems on the same block, e.g. multiple phases
  const std::string _base_name;

  /// Elasticity tensor as defined by a separate class
  const MaterialProperty<RankFourTensor> & _elasticity_tensor;

  ///Maximum number of active slip systems for the crystalline material being modeled
  const unsigned int _number_slip_systems;

  /// File should contain slip plane normal and direction.
  std::string _slip_sys_file_name;

  /// @{Parameters to characterize the cross slip behavior of the crystal
  const Real _number_cross_slip_directions;
  const Real _number_cross_slip_planes;
  ///@}

  /// Stress residual equation relative tolerance
  Real _rtol;
  /// Stress residual equation absolute tolerance
  Real _abs_tol;
  /// Internal variable update equation tolerance
  Real _rel_state_var_tol;
  /// Slip increment tolerance
  Real _slip_incr_tol;
  /// Tolerance for change in slip system resistance over an increment
  Real _resistance_tol;
  /// Residual tolerance when variable value is zero. Default 1e-12.
  Real _zero_tol;

  /// Residual tensor
  RankTwoTensor _residual_tensor;
  /// Jacobian tensor
  RankFourTensor _jacobian;

  /// Maximum number of iterations for stress update
  unsigned int _maxiter;
  /// Maximum number of iterations for internal variable update
  unsigned int _maxiterg;

  /// Type of tangent moduli calculation
  MooseEnum _tan_mod_type;

  /// Maximum number of substep iterations
  unsigned int _max_substep_iter;

  /// Flag to activate line serach
  bool _use_line_search;

  /// Minimum line search step size
  Real _min_line_search_step_size;

  /// Line search bisection method tolerance
  Real _line_search_tolerance;

  /// Line search bisection method maximum iteration number
  unsigned int _line_search_max_iterations;

  /// strain formulation
  enum class LineSearchMethod
  {
    CutHalf,
    Bisection
  } _line_search_method;

  ///@{Plastic deformation gradient RankTwoTensor for the crystal
  MaterialProperty<RankTwoTensor> & _plastic_deformation_gradient;
  const MaterialProperty<RankTwoTensor> & _plastic_deformation_gradient_old;
  ///@}

  ///@{Total deformation gradient RankTwoTensor for the crystal
  const MaterialProperty<RankTwoTensor> & _deformation_gradient;
  const MaterialProperty<RankTwoTensor> & _deformation_gradient_old;
  ///@}

  ///@{Second Piola-Kirchoff stress measure
  MaterialProperty<RankTwoTensor> & _pk2;
  const MaterialProperty<RankTwoTensor> & _pk2_old;
  ///@}

  /// Lagrangian total strain measure for the entire crystal
  MaterialProperty<RankTwoTensor> & _total_lagrangian_strain;

  ///@{Slip system direction and normal and associated Schmid tensors
  DenseVector<Real> _slip_direction;
  DenseVector<Real> _slip_plane_normal;
  MaterialProperty<std::vector<RankTwoTensor>> & _flow_direction;
  ///@}

  /// Resolved shear stress on each slip system
  MaterialProperty<std::vector<Real>> & _tau;

  /**
   * Tracks the rotation of the crystal during deformation
   * Note: this rotation tensor is not applied to the crystal lattice
   */
  MaterialProperty<RankTwoTensor> & _update_rotation;

  /**
   * Crystal rotation in the original, or reference, configuration as defined by
   * Euler angle arguments in the ComputeElasticityTensor classes
   */
  const MaterialProperty<RankTwoTensor> & _crysrot;

  ///@{Helper deformation gradient tensor variables used in iterative solve
  RankTwoTensor _temporary_deformation_gradient;
  RankTwoTensor _elastic_deformation_gradient;
  RankTwoTensor _inverse_plastic_deformation_grad;
  RankTwoTensor _inverse_plastic_deformation_grad_old;
  ///@}

  /// Flag to check whether convergence is achieved or if substepping is needed
  bool _error_tolerance;

  /// Substepping time step value used within the inheriting constitutive models
  Real _substep_dt;

  ///@{ Used for substepping; Uniformly divides the increment in deformation gradient
  RankTwoTensor _delta_deformation_gradient;
  RankTwoTensor _temporary_deformation_gradient_old;
  ///@}

  /// Scales the substepping increment to obtain deformation gradient at a substep iteration
  Real _dfgrd_scale_factor;

  /// Sorted slip system indices into cross slip family groups
  std::vector<std::vector<unsigned int>> _cross_slip_familes;

  /// Flag to run the cross slip calculations if cross slip numbers are specified
  bool _calculate_cross_slip;
};
