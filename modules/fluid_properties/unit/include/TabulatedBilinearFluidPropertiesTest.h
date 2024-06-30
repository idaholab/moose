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
#include "TabulatedBilinearFluidProperties.h"
#include "CO2FluidProperties.h"

class CO2FluidProperties;
class TabulatedBilinearFluidProperties;

class TabulatedBilinearFluidPropertiesTest : public MooseObjectUnitTest
{
public:
  TabulatedBilinearFluidPropertiesTest() : MooseObjectUnitTest("FluidPropertiesApp") { buildObjects(); }

protected:
  void buildObjects()
  {
    InputParameters co2_uo_params = _factory.getValidParams("CO2FluidProperties");
    _fe_problem->addUserObject("CO2FluidProperties", "co2_fp", co2_uo_params);
    _co2_fp = &_fe_problem->getUserObject<CO2FluidProperties>("co2_fp");

    InputParameters tab_uo_params = _factory.getValidParams("TabulatedBilinearFluidProperties");
    tab_uo_params.set<UserObjectName>("fp") = "co2_fp";
    tab_uo_params.set<FileName>("fluid_property_file") = "data/csv/fluid_props.csv";
    _fe_problem->addUserObject("TabulatedBilinearFluidProperties", "tab_fp", tab_uo_params);
    _tab_fp = &_fe_problem->getUserObject<TabulatedBilinearFluidProperties>("tab_fp");

    InputParameters tab_uo_ve_params = _factory.getValidParams("TabulatedBilinearFluidProperties");
    tab_uo_ve_params.set<UserObjectName>("fp") = "co2_fp";
    tab_uo_ve_params.set<bool>("construct_pT_from_ve") = true;
    tab_uo_ve_params.set<bool>("construct_pT_from_vh") = true;
    // Need to set these parameters to fit well within the provided data
    tab_uo_ve_params.set<Real>("temperature_min") = 445;
    tab_uo_ve_params.set<Real>("temperature_max") = 455;
    tab_uo_ve_params.set<Real>("pressure_min") = 1.45e6;
    tab_uo_ve_params.set<Real>("pressure_max") = 1.55e6;
    tab_uo_ve_params.set<Real>("T_initial_guess") = 450;
    tab_uo_ve_params.set<Real>("p_initial_guess") = 1.5e6;
    tab_uo_ve_params.set<FileName>("fluid_property_file") = "data/csv/fluid_props.csv";
    _fe_problem->addUserObject("TabulatedBilinearFluidProperties", "tab_fp_ve", tab_uo_ve_params);
    _tab_fp_ve = &_fe_problem->getUserObject<TabulatedBilinearFluidProperties>("tab_fp_ve");

    InputParameters tab_gen_uo_params = _factory.getValidParams("TabulatedBilinearFluidProperties");
    tab_gen_uo_params.set<UserObjectName>("fp") = "co2_fp";
    tab_gen_uo_params.set<Real>("temperature_min") = 400;
    tab_gen_uo_params.set<Real>("temperature_max") = 500;
    tab_gen_uo_params.set<Real>("pressure_min") = 1e6;
    tab_gen_uo_params.set<Real>("pressure_max") = 2e6;
    tab_gen_uo_params.set<unsigned int>("num_T") = 6;
    tab_gen_uo_params.set<unsigned int>("num_p") = 6;
    MultiMooseEnum properties("density enthalpy internal_energy viscosity k cv cp entropy");
    tab_gen_uo_params.set<MultiMooseEnum>("interpolated_properties") = properties;
    _fe_problem->addUserObject("TabulatedBilinearFluidProperties", "tab_gen_fp", tab_gen_uo_params);
    _tab_gen_fp = &_fe_problem->getUserObject<TabulatedBilinearFluidProperties>("tab_gen_fp");

    InputParameters unordered_uo_params = _factory.getValidParams("TabulatedBilinearFluidProperties");
    unordered_uo_params.set<UserObjectName>("fp") = "co2_fp";
    unordered_uo_params.set<FileName>("fluid_property_file") = "data/csv/unordered_fluid_props.csv";
    _fe_problem->addUserObject("TabulatedBilinearFluidProperties", "unordered_fp", unordered_uo_params);
    _unordered_fp = &_fe_problem->getUserObject<TabulatedBilinearFluidProperties>("unordered_fp");

    InputParameters unequal_uo_params = _factory.getValidParams("TabulatedBilinearFluidProperties");
    unequal_uo_params.set<UserObjectName>("fp") = "co2_fp";
    unequal_uo_params.set<FileName>("fluid_property_file") = "data/csv/unequal_fluid_props.csv";
    _fe_problem->addUserObject("TabulatedBilinearFluidProperties", "unequal_fp", unequal_uo_params);
    _unequal_fp = &_fe_problem->getUserObject<TabulatedBilinearFluidProperties>("unequal_fp");

    InputParameters missing_col_uo_params = _factory.getValidParams("TabulatedBilinearFluidProperties");
    missing_col_uo_params.set<UserObjectName>("fp") = "co2_fp";
    missing_col_uo_params.set<FileName>("fluid_property_file") =
        "data/csv/missing_col_fluid_props.csv";
    _fe_problem->addUserObject("TabulatedBilinearFluidProperties", "missing_col_fp", missing_col_uo_params);
    _missing_col_fp = &_fe_problem->getUserObject<TabulatedBilinearFluidProperties>("missing_col_fp");

    InputParameters unknown_col_uo_params = _factory.getValidParams("TabulatedBilinearFluidProperties");
    unknown_col_uo_params.set<UserObjectName>("fp") = "co2_fp";
    unknown_col_uo_params.set<FileName>("fluid_property_file") = "data/csv/unknown_fluid_props.csv";
    _fe_problem->addUserObject("TabulatedBilinearFluidProperties", "unknown_col_fp", unknown_col_uo_params);
    _unknown_col_fp = &_fe_problem->getUserObject<TabulatedBilinearFluidProperties>("unknown_col_fp");

    InputParameters missing_data_uo_params = _factory.getValidParams("TabulatedBilinearFluidProperties");
    missing_data_uo_params.set<UserObjectName>("fp") = "co2_fp";
    missing_data_uo_params.set<FileName>("fluid_property_file") =
        "data/csv/missing_data_fluid_props.csv";
    _fe_problem->addUserObject(
        "TabulatedBilinearFluidProperties", "missing_data_fp", missing_data_uo_params);
    _missing_data_fp =
        &_fe_problem->getUserObject<TabulatedBilinearFluidProperties>("missing_data_fp");
  }

  void TearDown()
  {
    // We always want to generate a new file in the generateTabulatedData test,
    // so make sure that any existing data file is deleted after testing
    std::remove("fluid_properties.csv");
  }

  const CO2FluidProperties * _co2_fp;
  const TabulatedBilinearFluidProperties * _tab_fp;
  const TabulatedBilinearFluidProperties * _tab_fp_ve;
  const TabulatedBilinearFluidProperties * _tab_gen_fp;
  const TabulatedBilinearFluidProperties * _unordered_fp;
  const TabulatedBilinearFluidProperties * _unequal_fp;
  const TabulatedBilinearFluidProperties * _missing_col_fp;
  const TabulatedBilinearFluidProperties * _unknown_col_fp;
  const TabulatedBilinearFluidProperties * _missing_data_fp;
};
