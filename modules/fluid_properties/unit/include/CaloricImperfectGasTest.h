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
#include "CaloricImperfectGas.h"
#include "IdealGasFluidProperties.h"

class CaloricImperfectGasTest : public MooseObjectUnitTest
{
public:
  CaloricImperfectGasTest() : MooseObjectUnitTest("FluidPropertiesApp") { buildObjects(); }

protected:
  void buildObjects()
  {
    std::vector<Real> T = {100, 200, 300, 400, 500};
    std::vector<Real> h = {2859400, 8578200, 17156400, 28594000, 42891000};
    std::vector<Real> e = {1825110, 5272540, 10342290, 17034360, 25348750};
    std::vector<Real> cp = {42891, 71485, 100079, 128673, 157267};
    std::vector<Real> cv = {26362.7, 42585.9, 58809.1, 75032.3, 91255.5};

    InputParameters default_fn_params = _factory.getValidParams("ConstantFunction");
    default_fn_params.set<Real>("value") = 1.0;
    _fe_problem->addFunction("ConstantFunction", "default_fn", default_fn_params);

    InputParameters h_fn_params = _factory.getValidParams("PiecewiseLinear");
    h_fn_params.set<std::vector<Real>>("x") = T;
    h_fn_params.set<std::vector<Real>>("y") = h;
    _fe_problem->addFunction("PiecewiseLinear", "h_fn", h_fn_params);

    InputParameters e_fn_params = _factory.getValidParams("PiecewiseLinear");
    e_fn_params.set<std::vector<Real>>("x") = T;
    e_fn_params.set<std::vector<Real>>("y") = e;
    _fe_problem->addFunction("PiecewiseLinear", "e_fn", e_fn_params);

    InputParameters cp_fn_params = _factory.getValidParams("PiecewiseLinear");
    cp_fn_params.set<std::vector<Real>>("x") = T;
    cp_fn_params.set<std::vector<Real>>("y") = cp;
    _fe_problem->addFunction("PiecewiseLinear", "cp_fn", cp_fn_params);

    InputParameters cv_fn_params = _factory.getValidParams("PiecewiseLinear");
    cv_fn_params.set<std::vector<Real>>("x") = T;
    cv_fn_params.set<std::vector<Real>>("y") = cv;
    _fe_problem->addFunction("PiecewiseLinear", "cv_fn", cv_fn_params);

    std::vector<Real> T2 = {100, 500};
    std::vector<Real> mu = {1, 6};
    std::vector<Real> k = {11, 1};

    InputParameters mu_fn_params = _factory.getValidParams("PiecewiseLinear");
    mu_fn_params.set<std::vector<Real>>("x") = T2;
    mu_fn_params.set<std::vector<Real>>("y") = mu;
    _fe_problem->addFunction("PiecewiseLinear", "mu_fn", mu_fn_params);

    InputParameters k_fn_params = _factory.getValidParams("PiecewiseLinear");
    k_fn_params.set<std::vector<Real>>("x") = T2;
    k_fn_params.set<std::vector<Real>>("y") = k;
    _fe_problem->addFunction("PiecewiseLinear", "k_fn", k_fn_params);

    InputParameters uo_pars = _factory.getValidParams("CaloricImperfectGas");
    uo_pars.set<Real>("molar_mass") = 0.002;
    uo_pars.set<FunctionName>("h") = "h_fn";
    uo_pars.set<FunctionName>("e") = "e_fn";
    uo_pars.set<FunctionName>("cp") = "cp_fn";
    uo_pars.set<FunctionName>("cv") = "cv_fn";
    uo_pars.set<FunctionName>("mu") = "mu_fn";
    uo_pars.set<FunctionName>("k") = "k_fn";
    uo_pars.set<Real>("min_temperature") = 100.0;
    uo_pars.set<Real>("max_temperature") = 500.0;
    _fe_problem->addUserObject("CaloricImperfectGas", "fp", uo_pars);
    _fp = &_fe_problem->getUserObject<CaloricImperfectGas>("fp");

    // Testing case where h(T) is not monotonic
    std::vector<Real> bad_h = {2859400, 8578200, 17156400, 8578200, 42891000};
    InputParameters bad_h_fn_params = _factory.getValidParams("PiecewiseLinear");
    bad_h_fn_params.set<std::vector<Real>>("x") = T;
    bad_h_fn_params.set<std::vector<Real>>("y") = bad_h;
    _fe_problem->addFunction("PiecewiseLinear", "bad_h_fn", bad_h_fn_params);

    InputParameters uo_pars_bad_h_fn = _factory.getValidParams("CaloricImperfectGas");
    uo_pars_bad_h_fn.set<Real>("molar_mass") = 0.002;
    uo_pars_bad_h_fn.set<FunctionName>("h") = "bad_h_fn";
    uo_pars_bad_h_fn.set<FunctionName>("e") = "e_fn";
    uo_pars_bad_h_fn.set<FunctionName>("cp") = "cp_fn";
    uo_pars_bad_h_fn.set<FunctionName>("cv") = "cv_fn";
    uo_pars_bad_h_fn.set<FunctionName>("mu") = "default_fn";
    uo_pars_bad_h_fn.set<FunctionName>("k") = "default_fn";
    uo_pars_bad_h_fn.set<Real>("min_temperature") = 100.0;
    uo_pars_bad_h_fn.set<Real>("max_temperature") = 500.0;
    _fe_problem->addUserObject("CaloricImperfectGas", "fp_bad_h_fn", uo_pars_bad_h_fn);
    _fp_bad_h_fn = &_fe_problem->getUserObject<CaloricImperfectGas>("fp_bad_h_fn");

    std::vector<Real> bad_T = {150, 200, 300, 400, 500};
    InputParameters bad_h_fn_params2 = _factory.getValidParams("PiecewiseLinear");
    bad_h_fn_params2.set<std::vector<Real>>("x") = bad_T;
    bad_h_fn_params2.set<std::vector<Real>>("y") = h;
    _fe_problem->addFunction("PiecewiseLinear", "bad_h_fn2", bad_h_fn_params2);

    InputParameters uo_pars_bad_h_fn2 = _factory.getValidParams("CaloricImperfectGas");
    uo_pars_bad_h_fn2.set<Real>("molar_mass") = 0.002;
    uo_pars_bad_h_fn2.set<FunctionName>("h") = "bad_h_fn2";
    uo_pars_bad_h_fn2.set<FunctionName>("e") = "e_fn";
    uo_pars_bad_h_fn2.set<FunctionName>("cp") = "cp_fn";
    uo_pars_bad_h_fn2.set<FunctionName>("cv") = "cv_fn";
    uo_pars_bad_h_fn2.set<FunctionName>("mu") = "default_fn";
    uo_pars_bad_h_fn2.set<FunctionName>("k") = "default_fn";
    uo_pars_bad_h_fn2.set<Real>("min_temperature") = 100.0;
    uo_pars_bad_h_fn2.set<Real>("max_temperature") = 500.0;
    _fe_problem->addUserObject("CaloricImperfectGas", "fp_bad_h_fn2", uo_pars_bad_h_fn2);
    _fp_bad_h_fn2 = &_fe_problem->getUserObject<CaloricImperfectGas>("fp_bad_h_fn2");

    // compare ideal gas and calorically imperfect gas
    InputParameters ideal_uo_pars = _factory.getValidParams("IdealGasFluidProperties");
    ideal_uo_pars.set<Real>("molar_mass") = 0.002;
    ideal_uo_pars.set<Real>("gamma") = 1.41;
    _fe_problem->addUserObject("IdealGasFluidProperties", "ideal_fp", ideal_uo_pars);
    _ideal = &_fe_problem->getUserObject<IdealGasFluidProperties>("ideal_fp");

    Real Ru = 8.31446261815324;
    Real Rs = Ru / 0.002;
    Real gamma = 1.41;
    Real cp_val = gamma / (gamma - 1) * Rs;
    Real cv_val = cp_val / gamma;
    T = {0.0, 500.0};
    h = {0.0, 500.0 * cp_val};
    e = {0.0, 500.0 * cv_val};
    cp = {cp_val, cp_val};
    cv = {cv_val, cv_val};
    InputParameters lin_h_fn_params = _factory.getValidParams("PiecewiseLinear");
    lin_h_fn_params.set<std::vector<Real>>("x") = T;
    lin_h_fn_params.set<std::vector<Real>>("y") = h;
    _fe_problem->addFunction("PiecewiseLinear", "lin_h_fn", lin_h_fn_params);

    InputParameters lin_e_fn_params = _factory.getValidParams("PiecewiseLinear");
    lin_e_fn_params.set<std::vector<Real>>("x") = T;
    lin_e_fn_params.set<std::vector<Real>>("y") = e;
    _fe_problem->addFunction("PiecewiseLinear", "lin_e_fn", lin_e_fn_params);

    InputParameters const_cp_fn_params = _factory.getValidParams("PiecewiseLinear");
    const_cp_fn_params.set<std::vector<Real>>("x") = T;
    const_cp_fn_params.set<std::vector<Real>>("y") = cp;
    _fe_problem->addFunction("PiecewiseLinear", "const_cp_fn", const_cp_fn_params);

    InputParameters const_cv_fn_params = _factory.getValidParams("PiecewiseLinear");
    const_cv_fn_params.set<std::vector<Real>>("x") = T;
    const_cv_fn_params.set<std::vector<Real>>("y") = cv;
    _fe_problem->addFunction("PiecewiseLinear", "const_cv_fn", const_cv_fn_params);

    InputParameters compare_uo_pars = _factory.getValidParams("CaloricImperfectGas");
    compare_uo_pars.set<Real>("molar_mass") = 0.002;
    compare_uo_pars.set<FunctionName>("h") = "lin_h_fn";
    compare_uo_pars.set<FunctionName>("e") = "lin_e_fn";
    compare_uo_pars.set<FunctionName>("cp") = "const_cp_fn";
    compare_uo_pars.set<FunctionName>("cv") = "const_cv_fn";
    compare_uo_pars.set<FunctionName>("mu") = "default_fn";
    compare_uo_pars.set<FunctionName>("k") = "default_fn";
    compare_uo_pars.set<Real>("min_temperature") = 100.0;
    compare_uo_pars.set<Real>("max_temperature") = 500.0;
    _fe_problem->addUserObject("CaloricImperfectGas", "compare_fp", compare_uo_pars);
    _compare_with_ideal = &_fe_problem->getUserObject<CaloricImperfectGas>("compare_fp");
  }

  const CaloricImperfectGas * _fp;
  const CaloricImperfectGas * _fp_bad_h_fn;
  const CaloricImperfectGas * _fp_bad_h_fn2;
  const IdealGasFluidProperties * _ideal;
  const CaloricImperfectGas * _compare_with_ideal;
};
