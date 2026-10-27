//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObjectUnitTest.h"
#include "TabulatedPVTFluidProperties.h"

class TabulatedPVTFluidPropertiesTest : public MooseObjectUnitTest
{
public:
  TabulatedPVTFluidPropertiesTest() : MooseObjectUnitTest("PorousFlowApp")
  {
    _fp = &buildFP("fp", "ignore");
    _fp_bounds = &buildFP("fp_bounds", "set_to_closest_bound");
    _fp_throw = &buildFP("fp_throw", "throw");
    _fp_declare_invalid = &buildFP("fp_declare_invalid", "declare_invalid");
    _fp_warn_invalid = &buildFP("fp_warn_invalid", "warn_invalid");
  }

protected:
  /// Valid parameters for a PVDO-style dead-oil table
  InputParameters validFPParams()
  {
    InputParameters params = _factory.getValidParams("TabulatedPVTFluidProperties");
    params.set<Real>("density_sc") = _rho_sc;
    params.set<std::vector<Real>>("pressure") = _p;
    params.set<std::vector<Real>>("formation_volume_factor") = _B;
    params.set<std::vector<Real>>("viscosity") = _mu;
    params.set<Real>("cv") = _cv;
    params.set<Real>("molar_mass") = _molar_mass;
    return params;
  }

  const TabulatedPVTFluidProperties & buildFP(const std::string & name,
                                              const std::string & out_of_bounds_behavior)
  {
    InputParameters params = validFPParams();
    params.set<MooseEnum>("out_of_bounds_behavior") = out_of_bounds_behavior;
    _fe_problem->addUserObject("TabulatedPVTFluidProperties", name, params);
    return _fe_problem->getUserObject<TabulatedPVTFluidProperties>(name);
  }

  const Real _rho_sc = 850.0;
  const std::vector<Real> _p = {2.0e6, 1.0e7, 2.0e7, 3.0e7};
  const std::vector<Real> _B = {1.062, 1.050, 1.038, 1.028};
  const std::vector<Real> _mu = {1.04e-3, 1.10e-3, 1.17e-3, 1.24e-3};
  const Real _cv = 2000.0;
  const Real _molar_mass = 0.2;

  /// out_of_bounds_behavior = ignore (linear extrapolation)
  const TabulatedPVTFluidProperties * _fp;
  /// out_of_bounds_behavior = set_to_closest_bound
  const TabulatedPVTFluidProperties * _fp_bounds;
  /// out_of_bounds_behavior = throw (the default)
  const TabulatedPVTFluidProperties * _fp_throw;
  /// out_of_bounds_behavior = declare_invalid
  const TabulatedPVTFluidProperties * _fp_declare_invalid;
  /// out_of_bounds_behavior = warn_invalid
  const TabulatedPVTFluidProperties * _fp_warn_invalid;
};
