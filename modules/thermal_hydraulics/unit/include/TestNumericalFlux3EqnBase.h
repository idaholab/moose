//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObjectUnitTest.h"
#include "ADNumericalFlux3EqnBase.h"
#include "IdealGasFluidProperties.h"
#include "FluidPropertiesTestUtils.h"

/**
 * Base class for testing numerical flux objects for the variable-area compressible Euler equations.
 */
class TestNumericalFlux3EqnBase : public MooseObjectUnitTest
{
public:
  TestNumericalFlux3EqnBase()
    : MooseObjectUnitTest("ThermalHydraulicsApp"),

      _fp_name("fp"),
      _fp(getFluidPropertiesObject())
  {
  }

protected:
  /**
   * Builds and gets the fluid properties user object
   */
  const SinglePhaseFluidProperties & getFluidPropertiesObject()
  {
    const std::string class_name = "IdealGasFluidProperties";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<Real>("gamma") = 1.4;
    params.set<Real>("molar_mass") = 11.640243719999999;
    _fe_problem->addUserObject(class_name, _fp_name, params);
    return _fe_problem->getUserObject<SinglePhaseFluidProperties>(_fp_name);
  }

  /**
   * Computes the conservative solution from the primitive solution
   *
   * @param[in] W   Primitive solution vector: {p, T, vel}
   * @param[in] A   Cross-sectional area
   */
  std::vector<ADReal> computeConservativeSolution(const std::vector<ADReal> & W,
                                                  const ADReal & A) const;

  /**
   * Computes the 1D flux vector from the primitive solution
   *
   * @param[in] W   Primitive solution vector: {p, T, vel}
   * @param[in] A   Cross-sectional area
   */
  std::vector<ADReal> computeFluxFromPrimitive(const std::vector<ADReal> & W,
                                               const ADReal & A) const;

  /**
   * Creates the flux object to be tested
   */
  virtual const ADNumericalFlux3EqnBase * createFluxObject() = 0;

  /**
   * Gets a vector of pairs of primitive solution vectors to use for symmetry test
   */
  virtual std::vector<std::pair<std::vector<ADReal>, std::vector<ADReal>>>
  getPrimitiveSolutionsSymmetryTest() const = 0;

  /**
   * Gets a vector of primitive solution vectors to use for consistency test
   */
  virtual std::vector<std::vector<ADReal>> getPrimitiveSolutionsConsistencyTest() const = 0;

  /**
   * Runs the consistency test(s)
   */
  void testConsistency();

  /**
   * Runs the symmetry test(s)
   */
  void testSymmetry();

  /// Fluid properties user object name
  const UserObjectName _fp_name;
  /// Fluid properties user object
  const SinglePhaseFluidProperties & _fp;

  /// Flux object to be tested
  const ADNumericalFlux3EqnBase * _flux;
};
