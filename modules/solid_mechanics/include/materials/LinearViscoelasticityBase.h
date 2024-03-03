//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeElasticityTensorBase.h"
#include "RankFourTensor.h"
#include "RankTwoTensor.h"

/**
 * This class is a base class for materials consisting of an assembly of linear springs and
 * dashpots. It represents the arrangement of the system (typically, in parallel or in series),
 * the mechanical  * properties of each individual spring and dashpot in the model, and the
 * internal strains associated with each dashpot.
 *
 * To use a linear viscoelastic model, the user must provide a LinearViscoelasticityManager, that
 * will initialize and update the internal time-stepping scheme at the beginning and the end of
 * each time step.
 *
 * To compute the stress associated with a linear viscoelastic model, the user must either use a
 * ComputeLinearViscoelasticStress material (for total small strain formulations), or a
 * ComputeMultipleInelasticStress material associated with a LinearViscoelasticStressUpdate material
 * if there are multiple sources of inelastic strains (this requires to use incremental strains).
 *
 * The rate-dependent problem is solved using an internal 1-step finite difference scheme.
 * The scheme itself depends only on the arrangement of the system, and not the individual
 * properties. The scheme acts upon a series of internal strain variables, stored in the
 * _viscous_strains material properties. At the start of each time step, the apparent
 * elastic properties of the assembly and the apparent creep strain are calculated. The
 * rate-dependent problem becomes then equivalent to a purely elastic system. At the end of the
 * time step, the internal strain variables are updated according to the current
 * values of the stress and the strain.
 *
 * The real system refers to
 *     stress = elasticity_tensor : (mechanical_strain - creep_strain)
 *
 * The internal time-stepping scheme uses the following conversion:
 *     stress = apparent_elasticity_tensor : (mechanical_strain - apparent_creep_strain)
 *
 * The creep strain is generally driven only by the mechanical stress, but it can be driven by an
 * eigenstrain in addition to the stress. This means that the creep strain is calculated as if the
 * eigenstrain were to contribute to the global mechanical stress of the system.
 *
 * The determination of the apparent properties, as well as the update of the internal strain
 * variables depend only on the arrangement of the system (in parralel or series), and not their
 * actual properties. These calculations are generally only performed at the beginning of the time
 * step, but the user (or derived classes) can enforce these to be performed at each step of the
 * nonlinear solver if necessary.
 *
 * The time-integration uses a 1-step Newmark finite difference scheme. This scheme is controlled
 * by a parameter theta (between 0 and 1, default-value 1). Theta can be automatically calibrated
 * depending on the value of the dashpot viscosities in order to reproduce the exact integral
 * of exponential series (integration_rule = "zienkiewicz"). Theta cannot be set to 0 (purely
 * explicit system), and setting theta < 0.5 may cause the scheme to diverge (the "backward_euler"
 * "mid_point" and "zienkiewicz" schemes are unconditionally stable).
 *
 * This class is virtual. Deriving this class must be done in two steps.
 * 1. Classes inheriting directly from LinearViscoelasticityBase must represent a distinct
 * spring-dashpot architecture (examples in GeneralizedKelvinVoigtBase and GeneralizedMaxwellBase),
 * and must inherit only methods related to the time-stepping scheme itself.
 * 2. Classes inheriting inderectly from LinearViscoelasticityBase must provide the actual
 * properties of each spring and dashpot in the assembly (examples in GeneralizedKelvinVoigtModel
 * and GeneralizedMaxwellModel). These classes must not override methods related to the
 * time-stepping scheme.
 *
 * This class does not store the true creep strain of the material, but an equivalent creep strain
 * adapted for numerical efficiency. The true creep strain itself is declared by
 * ComputeLinearViscoelasticStress or LinearViscoelasticStressUpdate.
 */
class LinearViscoelasticityBase : public ComputeElasticityTensorBase
{
public:
  /**
   * Determines how theta is calculated for the time-integration system.
   */
  enum class IntegrationRule
  {
    /// theta = 1
    BackwardEuler,
    /// theta = 0.5
    MidPoint,
    /// theta defined by the user
    Newmark,
    /// theta automatically adjusted as a function of the time step and the viscosity
    Zienkiewicz,
  };

  static InputParameters validParams();

  LinearViscoelasticityBase(const InputParameters & parameters);

  /**
   * Compute the apparent properties at a quadrature point. This initializes the internal
   * time-stepping scheme, and must be called at the beginning of the time step.
   *
   * This method is called by LinearViscoelasticityManager.
   */
  void recomputeQpApparentProperties(unsigned int qp);

protected:
  virtual void initQpStatefulProperties() override;
  /// Inherited from ComputeElasticityTensorBase
  virtual void computeQpElasticityTensor() final;

  /**
   * This method assigns the mechanical properties of each spring and dashpot in the system.
   *
   * This method is purely virtual. Inherited classes must override it.
   *
   * This method is related to the storage of the mechanical properties of each spring and dashpot
   * in the system, and not the internal time-stepping procedure. Only end-user classes should
   * override it. See GeneralizedKelvinVoigModel for example.
   */
  virtual void computeQpViscoelasticProperties() = 0;

  /**
   * This method computes the inverse elasticity tensor of each spring in the system (if required).
   *
   * This method is virtual. Its default behavior computes the inverse of each tensor. It must be
   * inherited only if there is a faster way to compute this inverse (for example, if they are
   * known).
   *
   * This method is related to the storage of the mechanical properties of each spring and dashpot
   * in the system, and not the internal time-stepping procedure. Only end-user classes should
   * override it. See GeneralizedKelvinVoigtModel for example.
   */
  virtual void computeQpViscoelasticPropertiesInv();

  /**
   * This method computes the apparent elasticity tensor used in the internal time-stepping scheme.
   * It is called after the mechanical properties have been set, and before the apparent creep
   * strains are calculated.
   *
   * This method is also responsible for calculating the instantaneous elasticity tensor, and the
   * inverse of both the apparent and instantaneous elasticity tensors.
   *
   * This method is purely virtual. Inherited classes must override it.
   *
   * This method is related to the internal time-stepping scheme. It should only be overwritten by
   * classes that inherit directly from LinearViscoelasticityBase, and that represent a different
   * spring-dashpot assembly. See GeneralizedKelvinVoigtBase for example.
   */
  virtual void computeQpApparentElasticityTensors() = 0;

  /**
   * This method computes the apparent creep strain corresponding to the current viscous_strain of
   * each dashpot. It must be called after the apparent elasticity tensors have been calculated.
   *
   * This method is purely virtual. Inherited classes must override it.
   *
   * This method is related to the internal time-stepping scheme. It should only be overwritten by
   * classes that inherit directly from LinearViscoelasticityBase, and that represent a different
   * spring-dashpot assembly. See GeneralizedKelvinVoigtBase for example.
   */
  virtual void computeQpApparentCreepStrain() = 0;

  /**
   * Update the internal viscous strains at a quadrature point. Calling this method is required at
   * the end of each time step to update the internal time-stepping scheme correctly.
   *
   * This method is pure virtual. Inherited classes must override it.
   *
   * This method is related to the internal time-stepping scheme. It should only be overwritten by
   * classes that inherit directly from LinearViscoelasticityBase, and that represent a different
   * spring-dashpot assembly. See GeneralizedKelvinVoigtBase or GeneralizedMaxwellBase for
   * example.
   */
  virtual void updateQpViscousStrains() = 0;

  /**
   * Declare all necessary MaterialProperties for the model. This method must be called once at
   * the end of the constructor of a final inherited class, after `_components` has been set.
   * See GeneralizedKelvinVoigtModel or GeneralizedMaxwell model for example.
   */
  void declareViscoelasticProperties();

  /// Provides theta as a function of the time step and a viscosity
  Real computeTheta(Real dt, Real viscosity) const;

  /// Determines how theta is computed
  IntegrationRule _integration_rule;
  /// User-defined value for theta
  Real _theta;

  /// Apparent elasticity tensor. This is NOT the elasticity tensor of the material
  MaterialProperty<RankFourTensor> & _apparent_elasticity_tensor;
  /// Inverse of the apparent elasticity tensor
  MaterialProperty<RankFourTensor> & _apparent_elasticity_tensor_inv;

  /// Instantaneous elasticity tensor. This IS the real elasticity tensor of the material
  //  MaterialProperty<RankFourTensor> & _instantaneous_elasticity_tensor;
  /// Inverse of the instaneous elasticity tensor
  MaterialProperty<RankFourTensor> & _elasticity_tensor_inv;

  /// If active, indicates that we need to call computeQpViscoelasticPropertiesInv()
  bool _need_viscoelastic_properties_inverse;
  /// Indicates if the spring-dashpot assembly has a single dashpot not associated with a spring
  bool _has_longterm_dashpot;
  /**
   * This is the number of internal variables required by the model. This must be set in the
   * constructor of an inherited class. See GeneralizedKelvinVoigtModel for example.
   */
  unsigned int _components;

  ///@{ Elasticity tensor of a stand-alone elastic spring in the chain
  MaterialProperty<RankFourTensor> & _first_elasticity_tensor;
  MaterialProperty<RankFourTensor> * _first_elasticity_tensor_inv;
  ///@}

  ///@{ List of elasticity tensor of each subsequent spring in the chain
  std::vector<MaterialProperty<RankFourTensor> *> _springs_elasticity_tensors;
  std::vector<MaterialProperty<RankFourTensor> *> _springs_elasticity_tensors_inv;
  std::vector<const MaterialProperty<RankFourTensor> *> _springs_elasticity_tensors_inv_old;
  ///@}

  ///@{ List of viscosities of each subsequent dashpot in the chain
  std::vector<MaterialProperty<Real> *> _dashpot_viscosities;
  std::vector<const MaterialProperty<Real> *> _dashpot_viscosities_old;
  ///@}

  /**@{
   * The internal strain variables required by the time-stepping procedure (must be on a
   * one-on-one basis with the number of dashpot).
   */
  std::vector<MaterialProperty<RankTwoTensor> *> _viscous_strains;
  std::vector<const MaterialProperty<RankTwoTensor> *> _viscous_strains_old;
  ///@}

  ///@{ The apparent creep strain resulting from the internal viscous strains
  MaterialProperty<RankTwoTensor> & _apparent_creep_strain;
  const MaterialProperty<RankTwoTensor> & _apparent_creep_strain_old;
  ///@}

  /// previous value of the elastic strain for update purposes
  const MaterialProperty<RankTwoTensor> & _elastic_strain_old;
  /**
   * Previous value of the true creep strain for update purposes.
   * This is calculated by a ComputeLinearViscoelasticStress or a
   * LinearViscoelasticStressUpdate material.
   */
  const MaterialProperty<RankTwoTensor> & _creep_strain_old;

  /// Indicates if the model is only driven by the stress, or also by an additional eigenstrain
  bool _has_driving_eigenstrain;
  /// Name of the eigenstrain that drives the additional creep strain
  std::string _driving_eigenstrain_name;
  ///@{ Pointer to the value of the driving eigenstrain
  const MaterialProperty<RankTwoTensor> * const _driving_eigenstrain;
  const MaterialProperty<RankTwoTensor> * const _driving_eigenstrain_old;
  ///@}

  /**
   * If activated, the time-stepping scheme will be re-initialized at each step of the solver. This
   * may be required for models in which the mechanical properties vary following other variables.
   * If the mechanical properties are constant through the time step, this can be set to false.
   */
  bool _force_recompute_properties;

  /// checks whether we are at the first time step
  bool & _step_zero;
};
