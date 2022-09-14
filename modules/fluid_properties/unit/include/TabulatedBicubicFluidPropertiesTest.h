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

class CO2FluidProperties;
class TabulatedBicubicFluidProperties;

class TabulatedBicubicFluidPropertiesTest : public MooseObjectUnitTest
{
public:
  TabulatedBicubicFluidPropertiesTest() : MooseObjectUnitTest("FluidPropertiesApp") { buildObjects(); }

protected:
  void buildObjects()
  {
    InputParameters co2_uo_params = _factory.getValidParams("CO2FluidProperties");
    _fe_problem->addUserObject("CO2FluidProperties", "co2_fp", co2_uo_params);
    _co2_fp = &_fe_problem->getUserObject<CO2FluidProperties>("co2_fp");

    InputParameters tab_uo_params = _factory.getValidParams("TabulatedBicubicFluidProperties");
    tab_uo_params.set<UserObjectName>("fp") = "co2_fp";
    tab_uo_params.set<FileName>("fluid_property_file") = "data/csv/fluid_props.csv";
    _fe_problem->addUserObject("TabulatedBicubicFluidProperties", "tab_fp", tab_uo_params);
    _tab_fp = &_fe_problem->getUserObject<TabulatedBicubicFluidProperties>("tab_fp");

    InputParameters tab_uo_ve_params = _factory.getValidParams("TabulatedBicubicFluidProperties");
    tab_uo_ve_params.set<UserObjectName>("fp") = "co2_fp";
    tab_uo_ve_params.set<bool>("construct_pT_from_ve") = true;
    tab_uo_ve_params.set<bool>("construct_pT_from_vh") = true;
    tab_uo_ve_params.set<FileName>("fluid_property_file") = "data/csv/fluid_props.csv";
    // Need to set these parameters to fit well within the provided data
    tab_uo_ve_params.set<Real>("temperature_min") = 440;
    tab_uo_ve_params.set<Real>("temperature_max") = 460;
    tab_uo_ve_params.set<Real>("pressure_min") = 1.4e6;
    tab_uo_ve_params.set<Real>("pressure_max") = 1.6e6;
    tab_uo_ve_params.set<Real>("T_initial_guess") = 450;
    tab_uo_ve_params.set<Real>("p_initial_guess") = 1.5e6;
    _fe_problem->addUserObject("TabulatedBicubicFluidProperties", "tab_fp_ve", tab_uo_ve_params);
    _tab_fp_ve = &_fe_problem->getUserObject<TabulatedBicubicFluidProperties>("tab_fp_ve");

    InputParameters tab_gen_uo_params = _factory.getValidParams("TabulatedBicubicFluidProperties");
    tab_gen_uo_params.set<UserObjectName>("fp") = "co2_fp";
    tab_gen_uo_params.set<Real>("temperature_min") = 400;
    tab_gen_uo_params.set<Real>("temperature_max") = 500;
    tab_gen_uo_params.set<Real>("pressure_min") = 1e6;
    tab_gen_uo_params.set<Real>("pressure_max") = 2e6;
    tab_gen_uo_params.set<unsigned int>("num_T") = 6;
    tab_gen_uo_params.set<unsigned int>("num_p") = 6;
    MultiMooseEnum properties("density enthalpy internal_energy viscosity k cv cp entropy");
    tab_gen_uo_params.set<MultiMooseEnum>("interpolated_properties") = properties;
    _fe_problem->addUserObject("TabulatedBicubicFluidProperties", "tab_gen_fp", tab_gen_uo_params);
    _tab_gen_fp = &_fe_problem->getUserObject<TabulatedBicubicFluidProperties>("tab_gen_fp");

    InputParameters unordered_uo_params = _factory.getValidParams("TabulatedBicubicFluidProperties");
    unordered_uo_params.set<UserObjectName>("fp") = "co2_fp";
    unordered_uo_params.set<FileName>("fluid_property_file") = "data/csv/unordered_fluid_props.csv";
    _fe_problem->addUserObject("TabulatedBicubicFluidProperties", "unordered_fp", unordered_uo_params);
    _unordered_fp = &_fe_problem->getUserObject<TabulatedBicubicFluidProperties>("unordered_fp");

    InputParameters unequal_uo_params = _factory.getValidParams("TabulatedBicubicFluidProperties");
    unequal_uo_params.set<UserObjectName>("fp") = "co2_fp";
    unequal_uo_params.set<FileName>("fluid_property_file") = "data/csv/unequal_fluid_props.csv";
    _fe_problem->addUserObject("TabulatedBicubicFluidProperties", "unequal_fp", unequal_uo_params);
    _unequal_fp = &_fe_problem->getUserObject<TabulatedBicubicFluidProperties>("unequal_fp");

    InputParameters missing_col_uo_params = _factory.getValidParams("TabulatedBicubicFluidProperties");
    missing_col_uo_params.set<UserObjectName>("fp") = "co2_fp";
    missing_col_uo_params.set<FileName>("fluid_property_file") =
        "data/csv/missing_col_fluid_props.csv";
    _fe_problem->addUserObject("TabulatedBicubicFluidProperties", "missing_col_fp", missing_col_uo_params);
    _missing_col_fp = &_fe_problem->getUserObject<TabulatedBicubicFluidProperties>("missing_col_fp");

    InputParameters unknown_col_uo_params = _factory.getValidParams("TabulatedBicubicFluidProperties");
    unknown_col_uo_params.set<UserObjectName>("fp") = "co2_fp";
    unknown_col_uo_params.set<FileName>("fluid_property_file") = "data/csv/unknown_fluid_props.csv";
    _fe_problem->addUserObject("TabulatedBicubicFluidProperties", "unknown_col_fp", unknown_col_uo_params);
    _unknown_col_fp = &_fe_problem->getUserObject<TabulatedBicubicFluidProperties>("unknown_col_fp");

    InputParameters missing_data_uo_params = _factory.getValidParams("TabulatedBicubicFluidProperties");
    missing_data_uo_params.set<UserObjectName>("fp") = "co2_fp";
    missing_data_uo_params.set<FileName>("fluid_property_file") =
        "data/csv/missing_data_fluid_props.csv";
    _fe_problem->addUserObject(
        "TabulatedBicubicFluidProperties", "missing_data_fp", missing_data_uo_params);
    _missing_data_fp =
        &_fe_problem->getUserObject<TabulatedBicubicFluidProperties>("missing_data_fp");
  }

  void TearDown()
  {
    // We always want to generate a new file in the generateTabulatedData test,
    // so make sure that any existing data file is deleted after testing
    std::remove("fluid_properties.csv");
  }

  const CO2FluidProperties * _co2_fp;
  const TabulatedBicubicFluidProperties * _tab_fp;
  const TabulatedBicubicFluidProperties * _tab_fp_ve;
  const TabulatedBicubicFluidProperties * _tab_gen_fp;
  const TabulatedBicubicFluidProperties * _unordered_fp;
  const TabulatedBicubicFluidProperties * _unequal_fp;
  const TabulatedBicubicFluidProperties * _missing_col_fp;
  const TabulatedBicubicFluidProperties * _unknown_col_fp;
  const TabulatedBicubicFluidProperties * _missing_data_fp;
};
