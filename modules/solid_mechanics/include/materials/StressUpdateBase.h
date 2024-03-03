//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Conversion.h"
#include "InputParameters.h"
#include "Material.h"

/**
 * TangentCalculationMethod is an enum that determines the calculation method for the tangent
 * operator. ELASTIC uses the elasticity tensor as the tangent operator: J = C. The elasticity
 * tensor does not need to be provided by the StressUpdateBase models in this case. FULL calculates
 * the full tangent operator tensor in each inherited class. The full tangent operator is then
 * combined in ComputeMultipleInelasicStress by J = J_1 * C^-1 * J_2 * C^-1 * ... J_N. PARTIAL
 * calculates the contribution to the tangent operator if the terms need to be combined before being
 * inverted by J = (J_1 + J_2 + ... J_N)^-1 * C.
 */
enum class TangentCalculationMethod
{
  ELASTIC,
  FULL,
  PARTIAL
};

/**
 * StressUpdateBase is a material that is not called by MOOSE because
 * of the compute=false flag set in the parameter list.  This class is a base class
 * for materials that perform some internal computational
 * procedure (such as an iterative return-mapping procedure) to compute an
 * admissible state (which is usually an admissible stress that lies on or
 * within the yield surface, as well as a set of internal parameters such as
 * plastic strains).  The computational procedure must return the admissible stress
 * and a decomposition of the applied strain into elastic and inelastic components.
 * All materials inheriting from this class must be called by a separate material,
 * such as ComputeMultipleInelasticStress
 */
template <bool is_ad, typename R2 = RankTwoTensor, typename R4 = RankFourTensor>
class StressUpdateBaseTempl : public Material
{
public:
  static InputParameters validParams();

  StressUpdateBaseTempl(const InputParameters & parameters);

  using Material::_current_elem;
  using Material::_dt;
  using Material::_q_point;
  using Material::_qp;

  using GR2 = GenericRankTwoTensor<is_ad>;
  using GSR2 = Moose::GenericType<R2, is_ad>;
  using GR4 = GenericRankFourTensor<is_ad>;
  using GSR4 = Moose::GenericType<R4, is_ad>;

  /**
   * Given a strain increment that results in a trial stress, perform some
   * procedure (such as an iterative return-mapping process) to produce
   * an admissible stress, an elastic strain increment and an inelastic
   * strain increment.
   * If _fe_problem.currentlyComputingJacobian() = true, then updateState also computes
   * d(stress)/d(strain) (or some approximation to it).
   *
   * This method is called by ComputeMultipleInelasticStress.
   * This method is pure virutal: all inheriting classes must overwrite this method.
   *
   * @param strain_increment Upon input: the strain increment.  Upon output: the elastic strain
   * increment
   * @param inelastic_strain_increment The inelastic_strain resulting from the interative procedure
   * @param rotation_increment The finite-strain rotation increment
   * @param stress_new Upon input: the trial stress that results from applying strain_increment as
   * an elastic strain.  Upon output: the admissible stress
   * @param stress_old The old value of stress
   * @param elasticity_tensor The elasticity tensor
   * @param compute_full_tangent_operator The calling routine would like the full consistent tangent
   * operator to be placed in tangent_operator, if possible.  This is irrelevant if
   * _fe_problem.currentlyComputingJacobian() = false
   * @param tangent_operator d(stress)/d(strain), or some approximation to it  If
   * compute_full_tangent_operator=false, then tangent_operator=elasticity_tensor is an appropriate
   * choice.  tangent_operator is only computed if _fe_problem.currentlyComputingJacobian() = true
   */
  virtual void
  updateState(GR2 & strain_increment,
              GR2 & inelastic_strain_increment,
              const GR2 & rotation_increment,
              GR2 & stress_new,
              const RankTwoTensor & stress_old,
              const GR4 & elasticity_tensor,
              const RankTwoTensor & elastic_strain_old,
              bool compute_full_tangent_operator = false,
              RankFourTensor & tangent_operator = StressUpdateBaseTempl<is_ad>::_identityTensor);

  /**
   * Similar to the updateState function, this method updates the strain and stress for one substep
   */
  virtual void updateStateSubstep(
      GR2 & /*strain_increment*/,
      GR2 & /*inelastic_strain_increment*/,
      const GR2 & /*rotation_increment*/,
      GR2 & /*stress_new*/,
      const RankTwoTensor & /*stress_old*/,
      const GR4 & /*elasticity_tensor*/,
      const RankTwoTensor & /*elastic_strain_old*/,
      bool compute_full_tangent_operator = false,
      RankFourTensor & tangent_operator = StressUpdateBaseTempl<is_ad>::_identityTensor);

  /// Sets the value of the global variable _qp for inheriting classes
  void setQp(unsigned int qp);

  /**
   * If updateState is not called during a timestep, this will be.  This method allows derived
   * classes to set internal parameters from their Old values, for instance
   */
  virtual void propagateQpStatefulProperties();

  /**
   * Does the model require the elasticity tensor to be isotropic?
   */
  virtual bool requiresIsotropicTensor() = 0;

  /**
   * Is the implmented model isotropic? The safe default is 'false'.
   */
  virtual bool isIsotropic() { return false; };

  virtual Real computeTimeStepLimit();

  virtual TangentCalculationMethod getTangentCalculationMethod();

  ///@{ Retained as empty methods to avoid a warning from Material.C in framework. These methods are unused in all inheriting classes and should not be overwritten.
  void resetQpProperties() final {}
  void resetProperties() final {}
  ///@}

  /**
   * Does the model include the infrastructure for substep decomposition of the
   * elastic strain initially used to calculate the trial stress guess
   * Inheriting classes which wish to use the substepping capability should
   * overwrite this method and set it to return true.
   */
  virtual bool substeppingCapabilityEnabled() { return false; }

  /**
   * Has the user requested usage of (possibly) implemented substepping capability for inelastic
   * models.
   */
  virtual bool substeppingCapabilityRequested() { return false; }

  /**
   * Given the elastic strain increment compute the number of substeps required
   * to bring a substepped trial stress guess distance from the yield surface
   * into the tolerance specified in the individual child class.
   */
  virtual int calculateNumberSubsteps(const GR2 & /*strain_increment*/) { return 1; }

  /**
   * Properly set up the incremental calculation storage of the stateful material
   * properties in the inheriting classes
   */
  virtual void
  storeIncrementalMaterialProperties(const unsigned int /*total_number_of_substeps*/){};

  /**
   * Reset material properties. Useful for substepping with inelastic models.
   */
  virtual void resetIncrementalMaterialProperties(){};

  /**
   * Compute the strain energy rate density for this inelastic model for the current step.
   * @param stress The stress tensor at the end of the step
   * @param strain_rate The strain rate at the end of the step
   * @return The computed strain energy rate density
   */
  virtual Real computeStrainEnergyRateDensity(
      const GenericMaterialProperty<RankTwoTensor, is_ad> & /*stress*/,
      const GenericMaterialProperty<RankTwoTensor, is_ad> & /*strain_rate*/)
  {
    mooseError(
        "The computation of strain energy rate density needs to be implemented by a child class");
    return 0.0;
  }

protected:
  /// Name used as a prefix for all material properties related to the stress update model.
  const std::string _base_name;

  static RankFourTensor _identityTensor;
};
typedef StressUpdateBaseTempl<false> StressUpdateBase;
typedef StressUpdateBaseTempl<true> ADStressUpdateBase;

template <bool is_ad, typename R2, typename R4>
RankFourTensor StressUpdateBaseTempl<is_ad, R2, R4>::_identityTensor =
    RankFourTensor(RankFourTensor::initIdentityFour);
