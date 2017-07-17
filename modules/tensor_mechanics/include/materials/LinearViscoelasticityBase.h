/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef LINEARVISCOELASTICITYBASE_H
#define LINEARVISCOELASTICITYBASE_H

#include "ComputeElasticityTensorBase.h"
#include "RankFourTensor.h"
#include "RankTwoTensor.h"

class LinearViscoelasticityBase;

template <>
InputParameters validParams<LinearViscoelasticityBase>();

/**
 * This class is a base class for materials consisting of an assembly of linear springs and dashpots.
 * It represents both the arrangement of the system (typically, in parallel or in series), as well
 * as the mechanical properties of each individual spring and dashpot in the model.
 *
 * The rate-dependent problem is solved using an internal 1-step finite difference
 * scheme. The scheme itself depends only on the arrangement of the system, and not the individual
 * properties. The scheme acts upon a series of internal strain variables, as defined by a 
 * ComputeLinearViscoelasticCreepStrain object. At the start of each time step, the apparent 
 * elastic properties of the assembly and the apparent creep strain are calculated. The rate-dependent
 * problem becomes then equivalent to a purely elastic system, which is solved normally by Moose.
 * At the end of the time step, the internal strain variables are updated according to the current
 * values of the stress and the strain.
 *
 * The creep strain is generally driven only by the mechanical stress, but it can be driven by an
 * eigenstrain in addition to the stress. This means that the creep strain is calculated as if the
 * eigenstrain were to contribute to the global mechanical stress of the system.
 * 
 * The determination of the apparent properties, as well as the update of the internal strain variables
 * depend only on the arrangement of the system (in parralel or series), and not their actual properties.
 * These calculations are generally only performed at the beginning of the time step, but the user (or
 * derived classes) can enforce these to be performed at each step of the nonlinear solver if necessary.
 *
 * The time-integration uses a 1-step Newmark finite difference scheme. This scheme is controlled
 * by a parameter theta (between 0 and 1, default-value 1). Theta can be automatically calibrated
 * depending on the value of the dashpot viscosities in order to reproduce the exact integral
 * of exponential series (integration_rule = "zienkiewicz"). Theta cannot be set to 0 (purely explicit 
 * system), and setting theta < 0.5 may cause the scheme to diverge (the "backward_euler" "mid_point" and
 * "zienkiewicz" schemes are unconditionally stable).
 *
 * This class is virtual. Deriving this class must be done in two steps.
 * 1. Classes inheriting directly from LinearViscoelasticityBase must represent a distinct spring-dashpot
 * architecture (examples in GeneralizedKelvinVoigtBase and GeneralizedMaxwellBase), and must inherit only
 * methods related to the time-stepping scheme itself.
 * 2. Classes inheriting inderectly from LinearViscoelasticityBase must provide the actual properties of
 * each spring and dashpot in the assembly (examples in GeneralizedKelvinVoigtModel and 
 * GeneralizedMaxwellModel). These classes must not override methods related to the time-stepping scheme.
 *
 * Calculations necessitate the presence in the simulation of one object of each of the following types: 
 * - ComputeLinearViscoelasticCreepStrain (to store the internal viscoelastic strains)
 * - ComputeLinearViscoelasticStress (to compute the stress, and update the viscoelastic strains)
 * - StressTimestepSetup (user object to ensure that the system is properly initialized at the beginning of
 * each time step)
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

  LinearViscoelasticityBase(const InputParameters & parameters);

  void recomputeQpApparentProperties(unsigned int qp);

  virtual void
  updateQpApparentProperties(unsigned int qp,
                             const RankTwoTensor & effective_strain,
                             const RankTwoTensor & effective_stress) = 0;

  RankTwoTensor computeQpCreepStrain(unsigned int qp,
                                     const RankTwoTensor & strain); 

  RankTwoTensor computeQpCreepStrainIncrement(unsigned int qp,
                                              const RankTwoTensor & strain_increment); 

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpElasticityTensor() final;
  virtual void computeQpViscoelasticProperties() = 0;
  virtual void computeQpViscoelasticPropertiesInv();
  virtual void computeQpApparentElasticityTensors() = 0;
  virtual void computeQpApparentCreepStrain() = 0;

  void fillIsotropicElasticityTensor(RankFourTensor & tensor,
                                     Real young_modulus,
                                     Real poisson_ratio) const;

  Real computeTheta(Real dt, Real viscosity) const;

  IntegrationRule _integration_rule;
  Real _theta;

  MaterialProperty<RankFourTensor> & _apparent_elasticity_tensor;
  MaterialProperty<RankFourTensor> & _apparent_elasticity_tensor_inv;

  MaterialProperty<RankFourTensor> & _instantaneous_elasticity_tensor;
  MaterialProperty<RankFourTensor> & _instantaneous_elasticity_tensor_inv;

  bool _need_viscoelastic_properties_inverse;
  bool _has_longterm_dashpot;
  unsigned int _components;

  MaterialProperty<RankFourTensor> & _first_elasticity_tensor;
  MaterialProperty<RankFourTensor> * _first_elasticity_tensor_inv;
  MaterialProperty<std::vector<RankFourTensor>> & _springs_elasticity_tensors;
  MaterialProperty<std::vector<RankFourTensor>> * _springs_elasticity_tensors_inv;
  MaterialProperty<std::vector<Real>> & _dashpot_viscosities;

  MaterialProperty<std::vector<RankTwoTensor>> & _viscous_strains;
  MaterialProperty<std::vector<RankTwoTensor>> & _viscous_strains_old;

  MaterialProperty<RankTwoTensor> & _apparent_creep_strain;
  MaterialProperty<RankTwoTensor> & _apparent_creep_strain_old;
  
  bool _has_driving_eigenstrain;
  std::string _driving_eigenstrain_name;
  const MaterialProperty<RankTwoTensor> * _driving_eigenstrain;

  bool _force_recompute_properties;

  bool & _step_zero;
};

#endif // LINEARVISCOELASTICITYBASE_H
