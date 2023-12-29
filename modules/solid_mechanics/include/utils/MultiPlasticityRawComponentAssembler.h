//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "TensorMechanicsPlasticModel.h"
#include "UserObjectInterface.h"

/**
 * MultiPlasticityRawComponentAssembler holds and computes yield functions,
 * flow directions, etc, for use in FiniteStrainMultiPlasticity
 *
 * Here there are a number of numbering systems.
 *
 * There are _num_models of plastic models, read from the input file.
 *    Each of these models can, in principal, contain multiple
 *    'internal surfaces'.
 *    Models are numbered from 0 to _num_models - 1.
 *
 * There are _num_surfaces surfaces.  This
 *    = sum_{plastic_models} (numberSurfaces in model)
 *    Evidently _num_surface >= _num_models
 *    Surfaces are numbered from 0 to _num_surfaces - 1.
 *
 * The std::vectors _model_given_surface, _model_surface_given_surface
 * and _surfaces_given_model allow translation between these
 */
class MultiPlasticityRawComponentAssembler : public UserObjectInterface
{
public:
  static InputParameters validParams();

  MultiPlasticityRawComponentAssembler(const MooseObject * moose_object);

  virtual ~MultiPlasticityRawComponentAssembler() {}

protected:
  const InputParameters & _params;

  /// Number of plastic models for this material
  unsigned int _num_models;

  /**
   * Number of surfaces within the plastic models.
   * For many situations this will be = _num_models
   * since each model will contain just one surface.
   * More generally it is >= _num_models.  For instance,
   * Mohr-Coulomb is a single model with 6 surfaces
   */
  unsigned int _num_surfaces;

  /// _surfaces_given_model[model_number] = vector of surface numbers for this model
  std::vector<std::vector<unsigned int>> _surfaces_given_model;

  /// Allows initial set of active constraints to be chosen optimally
  MooseEnum _specialIC;

  /// User objects that define the yield functions, flow potentials, etc
  std::vector<const TensorMechanicsPlasticModel *> _f;

  /**
   * The active yield function(s)
   * @param stress the stress at which to calculate the yield function
   * @param intnl vector of internal parameters
   * @param active set of active constraints - only the active yield functions are put into "f"
   * @param[out] f the yield function (or functions in the case of multisurface plasticity)
   */
  virtual void yieldFunction(const RankTwoTensor & stress,
                             const std::vector<Real> & intnl,
                             const std::vector<bool> & active,
                             std::vector<Real> & f);

  /**
   * The derivative of the active yield function(s) with respect to stress
   * @param stress the stress at which to calculate the yield function
   * @param intnl vector of internal parameters
   * @param active set of active constraints - only the active derivatives are put into "df_dstress"
   * @param[out] df_dstress the derivative (or derivatives in the case of multisurface plasticity).
   * df_dstress[alpha](i, j) = dyieldFunction[alpha]/dstress(i, j)
   */
  virtual void dyieldFunction_dstress(const RankTwoTensor & stress,
                                      const std::vector<Real> & intnl,
                                      const std::vector<bool> & active,
                                      std::vector<RankTwoTensor> & df_dstress);

  /**
   * The derivative of active yield function(s) with respect to their internal parameters (the user
   * objects assume there is exactly one internal param per yield function)
   * @param stress the stress at which to calculate the yield function
   * @param intnl vector of internal parameters
   * @param active set of active constraints - only the active derivatives are put into "df_dintnl"
   * @param[out] df_dintnl the derivatives.  df_dstress[alpha] = dyieldFunction[alpha]/dintnl[alpha]
   */
  virtual void dyieldFunction_dintnl(const RankTwoTensor & stress,
                                     const std::vector<Real> & intnl,
                                     const std::vector<bool> & active,
                                     std::vector<Real> & df_dintnl);

  /**
   * The active flow potential(s) - one for each yield function
   * @param stress the stress at which to calculate the flow potential
   * @param intnl vector of internal parameters
   * @param active set of active constraints - only the active flow potentials are put into "r"
   * @param[out] r the flow potential (flow potentials in the multi-surface case)
   */
  virtual void flowPotential(const RankTwoTensor & stress,
                             const std::vector<Real> & intnl,
                             const std::vector<bool> & active,
                             std::vector<RankTwoTensor> & r);

  /**
   * The derivative of the active flow potential(s) with respect to stress
   * @param stress the stress at which to calculate the flow potential
   * @param intnl vector of internal parameters
   * @param active set of active constraints - only the active derivatives are put into "dr_dstress"
   * @param[out] dr_dstress the derivative.  dr_dstress[alpha](i, j, k, l) = dr[alpha](i,
   * j)/dstress(k, l)
   */
  virtual void dflowPotential_dstress(const RankTwoTensor & stress,
                                      const std::vector<Real> & intnl,
                                      const std::vector<bool> & active,
                                      std::vector<RankFourTensor> & dr_dstress);

  /**
   * The derivative of the active flow potentials with respect to the active internal parameters
   * The UserObjects explicitly assume that r[alpha] is only dependent on intnl[alpha]
   * @param stress the stress at which to calculate the flow potential
   * @param intnl vector of internal parameters
   * @param active set of active constraints - only the active derivatives are put into "dr_dintnl"
   * @param[out] dr_dintnl the derivatives.  dr_dintnl[alpha](i, j) = dr[alpha](i, j)/dintnl[alpha]
   */
  virtual void dflowPotential_dintnl(const RankTwoTensor & stress,
                                     const std::vector<Real> & intnl,
                                     const std::vector<bool> & active,
                                     std::vector<RankTwoTensor> & dr_dintnl);

  /**
   * The active hardening potentials (one for each internal parameter and for each yield function)
   * by assumption in the Userobjects, the h[a][alpha] is nonzero only if the surface alpha is part
   * of model a, so we only calculate those here
   * @param stress the stress at which to calculate the hardening potential
   * @param intnl vector of internal parameters
   * @param active set of active constraints - only the active hardening potentials are put into "h"
   * @param[out] h the hardening potentials.  h[alpha] = hardening potential for yield fcn alpha
   * (and, by the above assumption we know which hardening parameter, a, this belongs to)
   */
  virtual void hardPotential(const RankTwoTensor & stress,
                             const std::vector<Real> & intnl,
                             const std::vector<bool> & active,
                             std::vector<Real> & h);

  /**
   * The derivative of the active hardening potentials with respect to stress
   * By assumption in the Userobjects, the h[a][alpha] is nonzero only for a = alpha, so we only
   * calculate those here
   * @param stress the stress at which to calculate the hardening potentials
   * @param intnl vector of internal parameters
   * @param active set of active constraints - only the active derivatives are put into "dh_dstress"
   * @param[out] dh_dstress the derivative.  dh_dstress[a](i, j) = dh[a]/dstress(k, l)
   */
  virtual void dhardPotential_dstress(const RankTwoTensor & stress,
                                      const std::vector<Real> & intnl,
                                      const std::vector<bool> & active,
                                      std::vector<RankTwoTensor> & dh_dstress);

  /**
   * The derivative of the active hardening potentials with respect to the active internal
   * parameters
   * @param stress the stress at which to calculate the hardening potentials
   * @param intnl vector of internal parameters
   * @param active set of active constraints - only the active derivatives are put into "dh_dintnl"
   * @param[out] dh_dintnl the derivatives.  dh_dintnl[a][alpha][b] = dh[a][alpha]/dintnl[b].  Note
   * that the userobjects assume that there is exactly one internal parameter per yield function, so
   * the derivative is only nonzero for a=alpha=b, so that is all we calculate
   */
  virtual void dhardPotential_dintnl(const RankTwoTensor & stress,
                                     const std::vector<Real> & intnl,
                                     const std::vector<bool> & active,
                                     std::vector<Real> & dh_dintnl);

  /**
   * Constructs a set of active constraints, given the yield functions, f.
   * This uses TensorMechanicsPlasticModel::activeConstraints to identify the active
   * constraints for each model.
   * @param f yield functions (should be _num_surfaces of these)
   * @param stress stress tensor
   * @param intnl internal parameters
   * @param Eijkl elasticity tensor (stress = Eijkl*strain)
   * @param[out] act the set of active constraints (will be resized to _num_surfaces)
   */
  virtual void buildActiveConstraints(const std::vector<Real> & f,
                                      const RankTwoTensor & stress,
                                      const std::vector<Real> & intnl,
                                      const RankFourTensor & Eijkl,
                                      std::vector<bool> & act);

  /// returns the model number, given the surface number
  unsigned int modelNumber(unsigned int surface);

  /// returns true if any internal surfaces of the given model are active according to 'active'
  bool anyActiveSurfaces(int model, const std::vector<bool> & active);

  /**
   * Returns the internal surface number(s) of the active surfaces of the given model
   * This may be of size=0 if there are no active surfaces of the given model
   * @param model the model number
   * @param active array with entries being 'true' if the surface is active
   * @param[out] active_surfaces_of_model the output
   */
  void activeModelSurfaces(int model,
                           const std::vector<bool> & active,
                           std::vector<unsigned int> & active_surfaces_of_model);

  /**
   * Returns the external surface number(s) of the active surfaces of the given model
   * This may be of size=0 if there are no active surfaces of the given model
   * @param model the model number
   * @param active array with entries being 'true' if the surface is active
   * @param[out] active_surfaces the output
   */
  void activeSurfaces(int model,
                      const std::vector<bool> & active,
                      std::vector<unsigned int> & active_surfaces);

  /**
   * Performs a returnMap for each plastic model using
   * their inbuilt returnMap functions.  This may be used
   * to quickly ascertain whether a (trial_stress, intnl_old) configuration
   * is admissible, or whether a single model's customized returnMap
   * function can provide a solution to the return-map problem,
   * or whether a full Newton-Raphson approach such as implemented
   * in ComputeMultiPlasticityStress is needed.
   *
   * There are three cases mentioned below:
   * (A) The (trial_stress, intnl_old) configuration is admissible
   *     according to all plastic models
   * (B) The (trial_stress, intnl_old) configuration is inadmissible
   *     to exactly one plastic model, and that model can successfully
   *     use its customized returnMap function to provide a returned
   *     (stress, intnl) configuration, and that configuration is
   *     admissible according to all plastic models
   * (C) All other cases.  This includes customized returnMap
   *     functions failing, or more than one plastic_model being
   *     inadmissible, etc
   *
   * @param trial_stress the trial stress
   * @param intnl_old the old values of the internal parameters
   * @param E_ijkl the elasticity tensor
   * @param ep_plastic_tolerance the tolerance on the plastic strain
   * @param[out] stress is set to trial_stress in case (A) or (C), and the returned value of stress
   * in case (B).
   * @param[out] intnl is set to intnl_old in case (A) or (C), and the returned value of intnl in
   * case (B)
   * @param[out] pm  Zero in case (A) or (C), otherwise the plastic multipliers needed to bring
   * about the returnMap in case (B)
   * @param[in/out] cumulative_pm   cumulative plastic multipliers, updated in case (B), otherwise
   * left untouched
   * @param[out] delta_dp is unchanged in case (A) or (C), and is set to the change in plastic
   * strain in case(B)
   * @param[out] yf will contain the yield function values at (stress, intnl)
   * @param[out] num_successful_plastic_returns will be 0 for (A) and (C), and 1 for (B)
   * @return true in case (A) and (B), and false in case (C)
   */
  bool returnMapAll(const RankTwoTensor & trial_stress,
                    const std::vector<Real> & intnl_old,
                    const RankFourTensor & E_ijkl,
                    Real ep_plastic_tolerance,
                    RankTwoTensor & stress,
                    std::vector<Real> & intnl,
                    std::vector<Real> & pm,
                    std::vector<Real> & cumulative_pm,
                    RankTwoTensor & delta_dp,
                    std::vector<Real> & yf,
                    unsigned & num_successful_plastic_returns,
                    unsigned & custom_model);

private:
  /// given a surface number, this returns the model number
  std::vector<unsigned int> _model_given_surface;

  /// given a surface number, this returns the corresponding-model's internal surface number
  std::vector<unsigned int> _model_surface_given_surface;

  /**
   * "Rock" version
   * Constructs a set of active constraints, given the yield functions, f.
   * This uses TensorMechanicsPlasticModel::activeConstraints to identify the active
   * constraints for each model.
   * @param f yield functions (should be _num_surfaces of these)
   * @param stress stress tensor
   * @param intnl internal parameters
   * @param Eijkl elasticity tensor (stress = Eijkl*strain)
   * @param[out] act the set of active constraints (will be resized to _num_surfaces)
   */
  void buildActiveConstraintsRock(const std::vector<Real> & f,
                                  const RankTwoTensor & stress,
                                  const std::vector<Real> & intnl,
                                  const RankFourTensor & Eijkl,
                                  std::vector<bool> & act);

  /**
   * "Joint" version
   * Constructs a set of active constraints, given the yield functions, f.
   * This uses TensorMechanicsPlasticModel::activeConstraints to identify the active
   * constraints for each model.
   * @param f yield functions (should be _num_surfaces of these)
   * @param stress stress tensor
   * @param intnl internal parameters
   * @param Eijkl elasticity tensor (stress = Eijkl*strain)
   * @param[out] act the set of active constraints (will be resized to _num_surfaces)
   */
  void buildActiveConstraintsJoint(const std::vector<Real> & f,
                                   const RankTwoTensor & stress,
                                   const std::vector<Real> & intnl,
                                   const RankFourTensor & Eijkl,
                                   std::vector<bool> & act);
};
