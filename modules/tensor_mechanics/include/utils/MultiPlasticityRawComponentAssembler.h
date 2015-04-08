/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef MULTIPLASTICITYRAWCOMPONENTASSEMBLER_H
#define MULTIPLASTICITYRAWCOMPONENTASSEMBLER_H

#include "TensorMechanicsPlasticModel.h"

class MultiPlasticityRawComponentAssembler;

template<>
InputParameters validParams<MultiPlasticityRawComponentAssembler>();

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
class MultiPlasticityRawComponentAssembler
{
public:
  MultiPlasticityRawComponentAssembler(const std::string & name, InputParameters parameters);

  virtual ~MultiPlasticityRawComponentAssembler() {}

protected:

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

  /// Allows initial set of active constraints to be chosen optimally
  MooseEnum _specialIC;

  /// User objects that define the yield functions, flow potentials, etc
  std::vector<const TensorMechanicsPlasticModel *> _f;

  /**
   * The active yield function(s)
   * @param stress the stress at which to calculate the yield function
   * @param intnl vector of internal parameters
   * @param active set of active constraints - only the active yield functions are put into "f"
   * @param num_active number of active constraints
   * @param f (output) the yield function (or functions in the case of multisurface plasticity)
   */
  virtual void yieldFunction(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<bool> & active, std::vector<Real> & f);


  /**
   * The derivative of the active yield function(s) with respect to stress
   * @param stress the stress at which to calculate the yield function
   * @param intnl vector of internal parameters
   * @param active set of active constraints - only the active derivatives are put into "df_dstress"
   * @param num_active number of active constraints
   * @param df_dstress (output) the derivative (or derivatives in the case of multisurface plasticity).  df_dstress[alpha](i, j) = dyieldFunction[alpha]/dstress(i, j)
   */
  virtual void dyieldFunction_dstress(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<bool> & active, std::vector<RankTwoTensor> & df_dstress);

  /**
   * The derivative of active yield function(s) with respect to their internal parameters (the user objects assume there is exactly one internal param per yield function)
   * @param stress the stress at which to calculate the yield function
   * @param intnl vector of internal parameters
   * @param active set of active constraints - only the active derivatives are put into "df_dintnl"
   * @param num_active number of active constraints
   * @param df_dintnl (output) the derivatives.  df_dstress[alpha] = dyieldFunction[alpha]/dintnl[alpha]
   */
  virtual void dyieldFunction_dintnl(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<bool> & active, std::vector<Real> & df_dintnl);

  /**
   * The active flow potential(s) - one for each yield function
   * @param stress the stress at which to calculate the flow potential
   * @param intnl vector of internal parameters
   * @param active set of active constraints - only the active flow potentials are put into "r"
   * @param num_active number of active constraints
   * @param r (output) the flow potential (flow potentials in the multi-surface case)
   */
  virtual void flowPotential(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<bool> & active, std::vector<RankTwoTensor> & r);

  /**
   * The derivative of the active flow potential(s) with respect to stress
   * @param stress the stress at which to calculate the flow potential
   * @param intnl vector of internal parameters
   * @param active set of active constraints - only the active derivatives are put into "dr_dstress"
   * @param num_active number of active constraints
   * @param dr_dstress (output) the derivative.  dr_dstress[alpha](i, j, k, l) = dr[alpha](i, j)/dstress(k, l)
   */
  virtual void dflowPotential_dstress(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<bool> & active, std::vector<RankFourTensor> & dr_dstress);

  /**
   * The derivative of the active flow potentials with respect to the active internal parameters
   * The UserObjects explicitly assume that r[alpha] is only dependent on intnl[alpha]
   * @param stress the stress at which to calculate the flow potential
   * @param intnl vector of internal parameters
   * @param active set of active constraints - only the active derivatives are put into "dr_dintnl"
   * @param num_active number of active constraints
   * @param dr_dintnl (output) the derivatives.  dr_dintnl[alpha](i, j) = dr[alpha](i, j)/dintnl[alpha]
   */
  virtual void dflowPotential_dintnl(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<bool> & active, std::vector<RankTwoTensor> & dr_dintnl);

  /**
   * The active hardening potentials (one for each internal parameter and for each yield function)
   * by assumption in the Userobjects, the h[a][alpha] is nonzero only for a = alpha, so we only calculate those here
   * @param stress the stress at which to calculate the hardening potential
   * @param intnl vector of internal parameters
   * @param active set of active constraints - only the active hardening potentials are put into "h"
   * @param num_active number of active constraints
   * @param h (output) the hardening potentials.  h[alpha] = hardening potential for yield fcn alpha and internal param a=alpha, by assumption in the userobjects this is only nonzero for a=alpha
   */
  virtual void hardPotential(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<bool> & active, std::vector<Real> & h);

  /**
   * The derivative of the active hardening potentials with respect to stress
   * By assumption in the Userobjects, the h[a][alpha] is nonzero only for a = alpha, so we only calculate those here
   * @param stress the stress at which to calculate the hardening potentials
   * @param intnl vector of internal parameters
   * @param active set of active constraints - only the active derivatives are put into "dh_dstress"
   * @param num_active number of active constraints
   * @param dh_dstress (output) the derivative.  dh_dstress[a](i, j) = dh[a]/dstress(k, l)
   */
  virtual void dhardPotential_dstress(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<bool> & active, std::vector<RankTwoTensor> & dh_dstress);

  /**
   * The derivative of the active hardening potentials with respect to the active internal parameters
   * @param stress the stress at which to calculate the hardening potentials
   * @param intnl vector of internal parameters
   * @param active set of active constraints - only the active derivatives are put into "dh_dintnl"
   * @param num_active number of active constraints
   * @param dh_dintnl (output) the derivatives.  dh_dintnl[a][alpha][b] = dh[a][alpha]/dintnl[b].  Note that the userobjects assume that there is exactly one internal parameter per yield function, so the derivative is only nonzero for a=alpha=b, so that is all we calculate
   */
  virtual void dhardPotential_dintnl(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<bool> & active, std::vector<Real> & dh_dintnl);

  /**
   * Constructs a set of active constraints, given the yield functions, f.
   * This uses TensorMechanicsPlasticModel::activeConstraints to identify the active
   * constraints for each model.
   * @param f yield functions (should be _num_surfaces of these)
   * @param stress stress tensor
   * @param intnl internal parameters
   * @param Eijkl elasticity tensor (stress = Eijkl*strain)
   * @param act (output) the set of active constraints (will be resized to _num_surfaces)
   */
  virtual void buildActiveConstraints(const std::vector<Real> & f, const RankTwoTensor & stress, const std::vector<Real> & intnl, const RankFourTensor & Eijkl, std::vector<bool> & act);

  /// returns the model number, given the surface number
  unsigned int modelNumber(unsigned int surface);

  /// returns true if any internal surfaces of the given model are active according to 'active'
  bool anyActiveSurfaces(int model, const std::vector<bool> & active);

  /**
   * Returns the internal surface number(s) of the active surfaces of the given model
   * This may be of size=0 if there are no active surfaces of the given model
   * @param model the model number
   * @param active array with entries being 'true' if the surface is active
   * @param active_surfaces_of_model (output) the output
   */
  void activeModelSurfaces(int model, const std::vector<bool> & active, std::vector<unsigned int> & active_surfaces_of_model);

  /**
   * Returns the external surface number(s) of the active surfaces of the given model
   * This may be of size=0 if there are no active surfaces of the given model
   * @param model the model number
   * @param active array with entries being 'true' if the surface is active
   * @param active_surfaces (output) the output
   */
  void activeSurfaces(int model, const std::vector<bool> & active, std::vector<unsigned int> & active_surfaces);

 private:

  /// given a surface number, this returns the model number
  std::vector<unsigned int> _model_given_surface;

  /// given a surface number, this returns the corresponding-model's internal surface number
  std::vector<unsigned int> _model_surface_given_surface;

  /// _surfaces_given_model[model_number] = vector of surface numbers for this model
  std::vector<std::vector<unsigned int> > _surfaces_given_model;

  /**
   * "Rock" version
   * Constructs a set of active constraints, given the yield functions, f.
   * This uses TensorMechanicsPlasticModel::activeConstraints to identify the active
   * constraints for each model.
   * @param f yield functions (should be _num_surfaces of these)
   * @param stress stress tensor
   * @param intnl internal parameters
   * @param Eijkl elasticity tensor (stress = Eijkl*strain)
   * @param act (output) the set of active constraints (will be resized to _num_surfaces)
   */
  void buildActiveConstraintsRock(const std::vector<Real> & f, const RankTwoTensor & stress, const std::vector<Real> & intnl, const RankFourTensor & Eijkl, std::vector<bool> & act);

  /**
   * "Joint" version
   * Constructs a set of active constraints, given the yield functions, f.
   * This uses TensorMechanicsPlasticModel::activeConstraints to identify the active
   * constraints for each model.
   * @param f yield functions (should be _num_surfaces of these)
   * @param stress stress tensor
   * @param intnl internal parameters
   * @param Eijkl elasticity tensor (stress = Eijkl*strain)
   * @param act (output) the set of active constraints (will be resized to _num_surfaces)
   */
  void buildActiveConstraintsJoint(const std::vector<Real> & f, const RankTwoTensor & stress, const std::vector<Real> & intnl, const RankFourTensor & Eijkl, std::vector<bool> & act);

};

#endif //MULTIPLASTICITYRAWCOMPONENTASSEMBLER_H
