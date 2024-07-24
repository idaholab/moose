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
#include "TabulatedBicubicFluidProperties.h"
#include "CO2FluidProperties.h"
#include "IdealGasFluidProperties.h"

class TabulatedBicubicFluidPropertiesTest : public MooseObjectUnitTest
{
public:
  TabulatedBicubicFluidPropertiesTest() : MooseObjectUnitTest("FluidPropertiesApp")
  {
    buildObjects();
  }

protected:
  void buildObjects()
  {
    InputParameters co2_uo_params = _factory.getValidParams("CO2FluidProperties");
    _fe_problem->addUserObject("CO2FluidProperties", "co2_fp", co2_uo_params);
    _co2_fp = &_fe_problem->getUserObject<CO2FluidProperties>("co2_fp");

    InputParameters idg_uo_params = _factory.getValidParams("IdealGasFluidProperties");
    _fe_problem->addUserObject("IdealGasFluidProperties", "idg_fp", idg_uo_params);
    _idg_fp = &_fe_problem->getUserObject<IdealGasFluidProperties>("idg_fp");

    InputParameters tab_uo_params = _factory.getValidParams("TabulatedBicubicFluidProperties");
    tab_uo_params.set<UserObjectName>("fp") = "co2_fp";
    _fe_problem->addUserObject("TabulatedBicubicFluidProperties", "tab_fp", tab_uo_params);
    _tab_pT_from_fp = &_fe_problem->getUserObject<TabulatedBicubicFluidProperties>("tab_fp");

    InputParameters tab_uo_ve_params = _factory.getValidParams("TabulatedBicubicFluidProperties");
    tab_uo_ve_params.set<bool>("construct_pT_from_ve") = true;
    tab_uo_ve_params.set<bool>("construct_pT_from_vh") = true;
    tab_uo_ve_params.set<FileName>("fluid_property_file") = "data/csv/fluid_props.csv";
    tab_uo_ve_params.set<Real>("T_initial_guess") = 450;
    tab_uo_ve_params.set<Real>("p_initial_guess") = 1.5e6;
    MultiMooseEnum properties("density enthalpy internal_energy viscosity k cv cp entropy",
                              "density enthalpy internal_energy viscosity k cv cp entropy");
    tab_uo_ve_params.set<MultiMooseEnum>("interpolated_properties") = properties;
    _fe_problem->addUserObject("TabulatedBicubicFluidProperties", "tab_fp_ve", tab_uo_ve_params);
    _tab_ve_from_pT = &_fe_problem->getUserObject<TabulatedBicubicFluidProperties>("tab_fp_ve");

    InputParameters tab_gen_uo_params = _factory.getValidParams("TabulatedBicubicFluidProperties");
    tab_gen_uo_params.set<UserObjectName>("fp") = "co2_fp";
    tab_gen_uo_params.set<Real>("temperature_min") = 400;
    tab_gen_uo_params.set<Real>("temperature_max") = 500;
    tab_gen_uo_params.set<Real>("pressure_min") = 1e6;
    tab_gen_uo_params.set<Real>("pressure_max") = 2e6;
    tab_gen_uo_params.set<unsigned int>("num_T") = 6;
    tab_gen_uo_params.set<unsigned int>("num_p") = 6;
    tab_gen_uo_params.set<MultiMooseEnum>("interpolated_properties") = properties;
    _fe_problem->addUserObject("TabulatedBicubicFluidProperties", "tab_gen_fp", tab_gen_uo_params);
    _tab_gen_fp = &_fe_problem->getUserObject<TabulatedBicubicFluidProperties>("tab_gen_fp");

    InputParameters tab_direct_ve_params =
        _factory.getValidParams("TabulatedBicubicFluidProperties");
    // We use ideal gas as it has more (v,e) support than co2
    tab_direct_ve_params.set<UserObjectName>("fp") = "idg_fp";
    tab_direct_ve_params.set<MooseEnum>("out_of_bounds_behavior") = "set_to_closest_bound";
    tab_direct_ve_params.set<bool>("create_pT_interpolations") = false;
    tab_direct_ve_params.set<bool>("create_ve_interpolations") = true;
    tab_direct_ve_params.set<Real>("temperature_min") = 400;
    tab_direct_ve_params.set<Real>("temperature_max") = 500;
    tab_direct_ve_params.set<Real>("pressure_min") = 1e6;
    tab_direct_ve_params.set<Real>("pressure_max") = 2e6;
    MultiMooseEnum properties_ve(
        "density enthalpy viscosity k c cv cp entropy pressure temperature",
        "density enthalpy viscosity k c cv cp entropy pressure temperature");
    tab_direct_ve_params.set<MultiMooseEnum>("interpolated_properties") = properties_ve;
    _fe_problem->addUserObject(
        "TabulatedBicubicFluidProperties", "tab_direct_ve", tab_direct_ve_params);
    _tab_ve_from_fp = &_fe_problem->getUserObject<TabulatedBicubicFluidProperties>("tab_direct_ve");

    // To test errors
    InputParameters unordered_uo_params =
        _factory.getValidParams("TabulatedBicubicFluidProperties");
    unordered_uo_params.set<FileName>("fluid_property_file") = "data/csv/unordered_fluid_props.csv";
    _fe_problem->addUserObject(
        "TabulatedBicubicFluidProperties", "unordered_fp", unordered_uo_params);
    _unordered_fp = &_fe_problem->getUserObject<TabulatedBicubicFluidProperties>("unordered_fp");

    InputParameters unequal_uo_params = _factory.getValidParams("TabulatedBicubicFluidProperties");
    unequal_uo_params.set<FileName>("fluid_property_file") = "data/csv/unequal_fluid_props.csv";
    _fe_problem->addUserObject("TabulatedBicubicFluidProperties", "unequal_fp", unequal_uo_params);
    _unequal_fp = &_fe_problem->getUserObject<TabulatedBicubicFluidProperties>("unequal_fp");

    InputParameters missing_col_uo_params =
        _factory.getValidParams("TabulatedBicubicFluidProperties");
    missing_col_uo_params.set<FileName>("fluid_property_file") =
        "data/csv/missing_col_fluid_props.csv";
    _fe_problem->addUserObject(
        "TabulatedBicubicFluidProperties", "missing_col_fp", missing_col_uo_params);
    _missing_col_fp =
        &_fe_problem->getUserObject<TabulatedBicubicFluidProperties>("missing_col_fp");

    InputParameters unknown_col_uo_params =
        _factory.getValidParams("TabulatedBicubicFluidProperties");
    unknown_col_uo_params.set<FileName>("fluid_property_file") = "data/csv/unknown_fluid_props.csv";
    _fe_problem->addUserObject(
        "TabulatedBicubicFluidProperties", "unknown_col_fp", unknown_col_uo_params);
    _unknown_col_fp =
        &_fe_problem->getUserObject<TabulatedBicubicFluidProperties>("unknown_col_fp");

    InputParameters missing_data_uo_params =
        _factory.getValidParams("TabulatedBicubicFluidProperties");
    missing_data_uo_params.set<FileName>("fluid_property_file") =
        "data/csv/missing_data_fluid_props.csv";
    _fe_problem->addUserObject(
        "TabulatedBicubicFluidProperties", "missing_data_fp", missing_data_uo_params);
    _missing_data_fp =
        &_fe_problem->getUserObject<TabulatedBicubicFluidProperties>("missing_data_fp");
  }

  // For generating reference data
  const CO2FluidProperties * _co2_fp;
  const IdealGasFluidProperties * _idg_fp;

  // For testing values
  const TabulatedBicubicFluidProperties * _tab_pT_from_fp;
  const TabulatedBicubicFluidProperties * _tab_ve_from_pT;
  const TabulatedBicubicFluidProperties * _tab_gen_fp;
  const TabulatedBicubicFluidProperties * _tab_ve_from_fp;

  // These properties are for testing errors and warnings
  const TabulatedBicubicFluidProperties * _unordered_fp;
  const TabulatedBicubicFluidProperties * _unequal_fp;
  const TabulatedBicubicFluidProperties * _missing_col_fp;
  const TabulatedBicubicFluidProperties * _unknown_col_fp;
  const TabulatedBicubicFluidProperties * _missing_data_fp;
};
