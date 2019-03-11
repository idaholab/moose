//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FLUIDPROPERTYINTERROGATOR_H
#define FLUIDPROPERTYINTERROGATOR_H

#include "GeneralUserObject.h"

class FluidPropertiesInterrogator;
class FluidProperties;
class LiquidFluidPropertiesInterface;
class SinglePhaseFluidProperties;
class VaporMixtureFluidProperties;
class TwoPhaseFluidProperties;
class TwoPhaseNCGFluidProperties;

template <>
InputParameters validParams<FluidPropertiesInterrogator>();

/**
 * User object for querying a single-phase or two-phase fluid properties object
 */
class FluidPropertiesInterrogator : public GeneralUserObject
{
public:
  FluidPropertiesInterrogator(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

protected:
  /**
   * Queries a 1-phase fluid properties object
   *
   * @param[in] fp_1phase     1-phase fluid properties
   * @param[in] description   String describing the 1-phase fluid properties
   * @param[in] throw_error_if_no_match   Option to throw an error if no sets
   *                                      match the inputs
   */
  void execute1Phase(const SinglePhaseFluidProperties * const fp_1phase,
                     const std::string & description,
                     bool throw_error_if_no_match);

  /**
   * Queries a vapor mixture fluid properties object
   *
   * @param[in] throw_error_if_no_match   Option to throw an error if no sets
   *                                      match the inputs
   */
  void executeVaporMixture(bool throw_error_if_no_match);

  /**
   * Queries a 2-phase fluid properties object
   */
  void execute2Phase();

  /**
   * Gets a map of a parameter set to a flag telling whether that set was provided
   *
   * @param[in] parameter_sets   vector of vectors of strings of parameter names.
   *                             The first dimension is the parameter set, and the second dimension
   *                             is a parameter within the set. If a set is a subset of another
   *                             set, the subset should be *after* the other.
   * @param[in] fp_type          string used to identify the set of parameter sets
   * @param[in] throw_error_if_no_match   Option to throw an error if no sets
   *                                      match the inputs
   */
  std::map<std::string, bool>
  getSpecifiedSetMap(const std::vector<std::vector<std::string>> & parameter_sets,
                     const std::string & fp_type,
                     bool throw_error_if_no_match) const;

  /**
   * Outputs a header for a section of properties
   */
  void outputHeader(const std::string & header) const;

  /**
   * Outputs a property value
   */
  void outputProperty(const std::string & name, const Real & value, const std::string & units);

  /**
   * Outputs static properties for a single-phase fluid properties object
   *
   * @param[in] fp    Single-phase fluid properties
   * @param[in] rho   Density
   * @param[in] e     Specific internal energy
   * @param[in] p     Pressure
   * @param[in] T     Temperature
   */
  void outputStaticProperties(const SinglePhaseFluidProperties * const fp,
                              const Real & rho,
                              const Real & e,
                              const Real & p,
                              const Real & T);

  /**
   * Outputs stagnation properties for a single-phase fluid properties object
   *
   * @param[in] fp    Single-phase fluid properties
   * @param[in] rho   Density
   * @param[in] e     Specific internal energy
   * @param[in] p     Pressure
   * @param[in] T     Temperature
   * @param[in] vel   Velocity
   */
  void outputStagnationProperties(const SinglePhaseFluidProperties * const fp,
                                  const Real & rho,
                                  const Real & e,
                                  const Real & p,
                                  const Real & T,
                                  const Real & vel);

  /**
   * Outputs liquid-specific properties
   *
   * @param[in] liquid_fp   Liquid fluid properties
   * @param[in] p   Pressure
   * @param[in] T   Temperature
   */
  void outputLiquidSpecificProperties(const LiquidFluidPropertiesInterface * const liquid_fp,
                                      const Real & p,
                                      const Real & T);

  /**
   * Outputs static properties for a vapor mixture fluid properties object
   *
   * @param[in] rho   Density
   * @param[in] e     Specific internal energy
   * @param[in] p     Pressure
   * @param[in] T     Temperature
   * @param[in] x_ncg   Mass fractions of NCGs
   */
  void outputVaporMixtureStaticProperties(const Real & rho,
                                          const Real & e,
                                          const Real & p,
                                          const Real & T,
                                          const std::vector<Real> & x_ncg);

  /**
   * Outputs stagnation properties for a vapor mixture fluid properties object
   *
   * @param[in] rho   Density
   * @param[in] e     Specific internal energy
   * @param[in] p     Pressure
   * @param[in] T     Temperature
   * @param[in] x_ncg   Mass fractions of NCGs
   * @param[in] vel   Velocity
   */
  void outputVaporMixtureStagnationProperties(const Real & rho,
                                              const Real & e,
                                              const Real & p,
                                              const Real & T,
                                              const std::vector<Real> & x_ncg,
                                              const Real & vel);

  /// pointer to fluid properties object
  const FluidProperties * const _fp;
  /// pointer to 1-phase fluid properties object (if provided 1-phase object)
  const SinglePhaseFluidProperties * const _fp_1phase;
  /// pointer to 2-phase fluid properties object (if provided 2-phase object)
  const TwoPhaseFluidProperties * const _fp_2phase;
  /// pointer to 2-phase NCG fluid properties object (if provided 2-phase NCG object)
  const TwoPhaseNCGFluidProperties * const _fp_2phase_ncg;

  /// flag that user provided 1-phase fluid properties
  const bool _has_1phase;
  /// flag that user provided vapor mixture fluid properties
  const bool _has_vapor_mixture;
  /// flag that user provided 2-phase fluid properties
  const bool _has_2phase;
  /// flag that user provided 2-phase NCG fluid properties
  const bool _has_2phase_ncg;

  /// pointer to liquid fluid properties object (if provided 2-phase object)
  const SinglePhaseFluidProperties * const _fp_liquid;
  /// pointer to vapor fluid properties object (if provided 2-phase object)
  const SinglePhaseFluidProperties * const _fp_vapor;
  /// pointer to vapor mixture fluid properties object (if provided 2-phase NCG object)
  const VaporMixtureFluidProperties * const _fp_vapor_mixture;

  /// flag that NaN has been encountered
  bool _nan_encountered;

  /// Precision used for printing values
  const unsigned int & _precision;
};

#endif /* FLUIDPROPERTYINTERROGATOR_H */
