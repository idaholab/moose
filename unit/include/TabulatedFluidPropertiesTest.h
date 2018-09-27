//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TABULATEDFLUIDPROPERTIESTEST_H
#define TABULATEDFLUIDPROPERTIESTEST_H

#include "MooseObjectUnitTest.h"
#include "TabulatedFluidProperties.h"
#include "CO2FluidProperties.h"

class CO2FluidProperties;
class TabulatedFluidProperties;

class TabulatedFluidPropertiesTest : public MooseObjectUnitTest
{
public:
  TabulatedFluidPropertiesTest() : MooseObjectUnitTest("MooseUnitApp") { buildObjects(); }

protected:
  void buildObjects()
  {
    InputParameters co2_uo_params = _factory.getValidParams("CO2FluidProperties");
    _fe_problem->addUserObject("CO2FluidProperties", "co2_fp", co2_uo_params);
    _co2_fp = &_fe_problem->getUserObject<CO2FluidProperties>("co2_fp");

    InputParameters tab_uo_params = _factory.getValidParams("TabulatedFluidProperties");
    tab_uo_params.set<UserObjectName>("fp") = "co2_fp";
    tab_uo_params.set<FileName>("fluid_property_file") = "data/csv/fluid_props.csv";
    _fe_problem->addUserObject("TabulatedFluidProperties", "tab_fp", tab_uo_params);
    _tab_fp = &_fe_problem->getUserObject<TabulatedFluidProperties>("tab_fp");

    InputParameters tab_gen_uo_params = _factory.getValidParams("TabulatedFluidProperties");
    tab_gen_uo_params.set<UserObjectName>("fp") = "co2_fp";
    tab_gen_uo_params.set<Real>("temperature_min") = 400;
    tab_gen_uo_params.set<Real>("temperature_max") = 500;
    tab_gen_uo_params.set<Real>("pressure_min") = 1e6;
    tab_gen_uo_params.set<Real>("pressure_max") = 2e6;
    tab_gen_uo_params.set<unsigned int>("num_T") = 6;
    tab_gen_uo_params.set<unsigned int>("num_p") = 6;
    MultiMooseEnum properties("density enthalpy internal_energy viscosity k cv cp entropy");
    tab_gen_uo_params.set<MultiMooseEnum>("interpolated_properties") = properties;
    _fe_problem->addUserObject("TabulatedFluidProperties", "tab_gen_fp", tab_gen_uo_params);
    _tab_gen_fp = &_fe_problem->getUserObject<TabulatedFluidProperties>("tab_gen_fp");

    InputParameters unordered_uo_params = _factory.getValidParams("TabulatedFluidProperties");
    unordered_uo_params.set<UserObjectName>("fp") = "co2_fp";
    unordered_uo_params.set<FileName>("fluid_property_file") = "data/csv/unordered_fluid_props.csv";
    _fe_problem->addUserObject("TabulatedFluidProperties", "unordered_fp", unordered_uo_params);
    _unordered_fp = &_fe_problem->getUserObject<TabulatedFluidProperties>("unordered_fp");

    InputParameters unequal_uo_params = _factory.getValidParams("TabulatedFluidProperties");
    unequal_uo_params.set<UserObjectName>("fp") = "co2_fp";
    unequal_uo_params.set<FileName>("fluid_property_file") = "data/csv/unequal_fluid_props.csv";
    _fe_problem->addUserObject("TabulatedFluidProperties", "unequal_fp", unequal_uo_params);
    _unequal_fp = &_fe_problem->getUserObject<TabulatedFluidProperties>("unequal_fp");

    InputParameters missing_col_uo_params = _factory.getValidParams("TabulatedFluidProperties");
    missing_col_uo_params.set<UserObjectName>("fp") = "co2_fp";
    missing_col_uo_params.set<FileName>("fluid_property_file") =
        "data/csv/missing_col_fluid_props.csv";
    _fe_problem->addUserObject("TabulatedFluidProperties", "missing_col_fp", missing_col_uo_params);
    _missing_col_fp = &_fe_problem->getUserObject<TabulatedFluidProperties>("missing_col_fp");

    InputParameters unknown_col_uo_params = _factory.getValidParams("TabulatedFluidProperties");
    unknown_col_uo_params.set<UserObjectName>("fp") = "co2_fp";
    unknown_col_uo_params.set<FileName>("fluid_property_file") = "data/csv/unknown_fluid_props.csv";
    _fe_problem->addUserObject("TabulatedFluidProperties", "unknown_col_fp", unknown_col_uo_params);
    _unknown_col_fp = &_fe_problem->getUserObject<TabulatedFluidProperties>("unknown_col_fp");

    InputParameters missing_data_uo_params = _factory.getValidParams("TabulatedFluidProperties");
    missing_data_uo_params.set<UserObjectName>("fp") = "co2_fp";
    missing_data_uo_params.set<FileName>("fluid_property_file") =
        "data/csv/missing_data_fluid_props.csv";
    _fe_problem->addUserObject(
        "TabulatedFluidProperties", "missing_data_fp", missing_data_uo_params);
    _missing_data_fp = &_fe_problem->getUserObject<TabulatedFluidProperties>("missing_data_fp");
  }

  void TearDown()
  {
    // We always want to generate a new file in the generateTabulatedData test,
    // so make sure that any existing data file is deleted after testing
    std::remove("fluid_properties.csv");
  }

  const CO2FluidProperties * _co2_fp;
  const TabulatedFluidProperties * _tab_fp;
  const TabulatedFluidProperties * _tab_gen_fp;
  const TabulatedFluidProperties * _unordered_fp;
  const TabulatedFluidProperties * _unequal_fp;
  const TabulatedFluidProperties * _missing_col_fp;
  const TabulatedFluidProperties * _unknown_col_fp;
  const TabulatedFluidProperties * _missing_data_fp;
};

#endif // TABULATEDFLUIDPROPERTIESTEST_H
