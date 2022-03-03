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

/**
 * Base class for testing rDG objects for the 3-equation model.
 */
class Test3EqnRDGObjectBase : public MooseObjectUnitTest
{
public:
  Test3EqnRDGObjectBase()
    : MooseObjectUnitTest("ThermalHydraulicsApp"),

      _fp_name("fp"),
      _fp(getFluidPropertiesObject()),

      _nLR_dot_d(1.0)
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
   * Creates the flux object to be tested
   */
  virtual const ADNumericalFlux3EqnBase * createFluxObject() = 0;

  /**
   * Runs the tests
   */
  virtual void test() = 0;

  /// Fluid properties user object name
  const UserObjectName _fp_name;
  /// Fluid properties user object
  const SinglePhaseFluidProperties & _fp;

  /// Dot product of normal from L to R with direction vector
  const Real _nLR_dot_d;

  /// Flux object to be tested
  const ADNumericalFlux3EqnBase * _flux;
};
