//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "gtest_include.h"

#include "GeochemicalSystem.h"

class GeochemicalSystemTest : public ::testing::Test
{
public:
  GeochemicalSystemTest()
    : _db_calcite("database/moose_testdb.json", true, true, false),
      _model_calcite(
          _db_calcite, {"H2O", "H+", "HCO3-", "Ca++"}, {"Calcite"}, {}, {}, {}, {}, "O2(aq)", "e-"),
      _model_kinetic_calcite(
          _db_calcite, {"H2O", "H+", "HCO3-", "Ca++"}, {}, {}, {"Calcite"}, {}, {}, "O2(aq)", "e-"),
      _mgd_calcite(_model_calcite.modelGeochemicalDatabase()),
      _mgd_kinetic_calcite(_model_kinetic_calcite.modelGeochemicalDatabase()),
      _swapper3(3, 1e-6),
      _swapper4(4, 1e-6),
      _swapper5(5, 1e-6),
      _swapper6(6, 1e-6),
      _swapper7(7, 1e-6),
      _swapper8(8, 1e-6),
      _cm_calcite({GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY,
                   GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
                   GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION,
                   GeochemicalSystem::ConstraintUserMeaningEnum::FREE_CONCENTRATION}),
      _cu_calcite({GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS,
                   GeochemistryUnitConverter::GeochemistryUnit::MOLES,
                   GeochemistryUnitConverter::GeochemistryUnit::MOLES,
                   GeochemistryUnitConverter::GeochemistryUnit::MOLAL}),
      _is3(3.0, 3.0, false, false),
      _ac3(_is3, _db_calcite),
      _is0(0.0, 0.0, false, false),
      _ac0(_is0, _db_calcite),
      _is8(1E-8, 1E-8, false, false),
      _ac8(_is8, _db_calcite),
      _egs_calcite(_mgd_calcite,
                   _ac3,
                   _is3,
                   _swapper4,
                   {"Ca++"},
                   {"Calcite"},
                   "H+",
                   {"H2O", "Calcite", "H+", "HCO3-"},
                   {1.75, 3.0, 2.0, 1.0},
                   _cu_calcite,
                   _cm_calcite,
                   25,
                   0,
                   1E-20,
                   {},
                   {},
                   {}),
      _egs_kinetic_calcite(_mgd_kinetic_calcite,
                           _ac3,
                           _is3,
                           _swapper4,
                           {},
                           {},
                           "H+",
                           {"H2O", "Ca++", "H+", "HCO3-"},
                           {1.75, 3.0, 2.0, 1.0},
                           _cu_calcite,
                           _cm_calcite,
                           25,
                           0,
                           1E-20,
                           {"Calcite"},
                           {1.1},
                           {GeochemistryUnitConverter::GeochemistryUnit::MOLES}),
      _cm_dummy({}),
      _cu_dummy({}),
      _model_redox(
          _db_calcite,
          {"H2O", "H+", "HCO3-", "O2(aq)", "(O-phth)--", "CH4(aq)", "StoiCheckRedox", "Fe+++"},
          {"Fe(OH)3(ppd)fake"},
          {"CH4(g)fake", "O2(g)"},
          {},
          {},
          {},
          "O2(aq)",
          "e-"),
      _mgd_redox(_model_redox.modelGeochemicalDatabase()),
      _cm_redox({GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY,
                 GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY,
                 GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY,
                 GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY,
                 GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY,
                 GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY,
                 GeochemicalSystem::ConstraintUserMeaningEnum::ACTIVITY,
                 GeochemicalSystem::ConstraintUserMeaningEnum::BULK_COMPOSITION}),
      _cu_redox({GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS,
                 GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS,
                 GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS,
                 GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS,
                 GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS,
                 GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS,
                 GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS,
                 GeochemistryUnitConverter::GeochemistryUnit::MOLES}),
      _egs_redox(
          _mgd_redox,
          _ac3,
          _is3,
          _swapper8,
          {},
          {},
          "Fe+++",
          {"H2O", "H+", "HCO3-", "O2(aq)", "(O-phth)--", "CH4(aq)", "StoiCheckRedox", "Fe+++"},
          {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0},
          _cu_redox,
          _cm_redox,
          25,
          0,
          1E-20,
          {},
          {},
          {})
  {
  }

protected:
  const GeochemicalDatabaseReader _db_calcite;
  const PertinentGeochemicalSystem _model_calcite;
  const PertinentGeochemicalSystem _model_kinetic_calcite;
  ModelGeochemicalDatabase _mgd_calcite;
  ModelGeochemicalDatabase _mgd_kinetic_calcite;
  GeochemistrySpeciesSwapper _swapper3;
  GeochemistrySpeciesSwapper _swapper4;
  GeochemistrySpeciesSwapper _swapper5;
  GeochemistrySpeciesSwapper _swapper6;
  GeochemistrySpeciesSwapper _swapper7;
  GeochemistrySpeciesSwapper _swapper8;
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> _cm_calcite;
  const std::vector<GeochemistryUnitConverter::GeochemistryUnit> _cu_calcite;
  GeochemistryIonicStrength _is3;
  GeochemistryActivityCoefficientsDebyeHuckel _ac3;
  GeochemistryIonicStrength _is0;
  GeochemistryActivityCoefficientsDebyeHuckel _ac0;
  GeochemistryIonicStrength _is8;
  GeochemistryActivityCoefficientsDebyeHuckel _ac8;
  const GeochemicalSystem _egs_calcite;
  const GeochemicalSystem _egs_kinetic_calcite;
  const std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> _cm_dummy;
  const std::vector<GeochemistryUnitConverter::GeochemistryUnit> _cu_dummy;
  const PertinentGeochemicalSystem _model_redox;
  ModelGeochemicalDatabase _mgd_redox;
  std::vector<GeochemicalSystem::ConstraintUserMeaningEnum> _cm_redox;
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> _cu_redox;
  const GeochemicalSystem _egs_redox;
};
