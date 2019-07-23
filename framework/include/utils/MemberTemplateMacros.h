//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MooseObject

#define adGetParam                                                                                 \
  _Pragma("GCC warning \"adGetParam is deprecated. Simply use getParam\"") this                    \
      ->template getParamTempl

#define getParam this->template getParamTempl

// UserObjectInterface

#define adGetUserObject                                                                            \
  _Pragma("GCC warning \"adGetUserObject is deprecated. Simply use getUserObject\"") this          \
      ->template getUserObjectTempl
#define adGetUserObjectByName                                                                      \
  _Pragma(                                                                                         \
      "GCC warning \"adGetUserObjectByName is deprecated. Simply use getUserObjectByName\"") this  \
      ->template getUserObjectByNameTempl

// doco-user-object-interface-begin
#define getUserObjectByName this->template getUserObjectByNameTempl
#define getUserObject this->template getUserObjectTempl
// doco-user-object-interface-end

// MaterialPropertyInterface

#define adGetMaterialProperty                                                                      \
  _Pragma(                                                                                         \
      "GCC warning \"adGetMaterialProperty is deprecated. Simply use getMaterialProperty\"") this  \
      ->template getMaterialPropertyTempl
// clang-format off
#define adGetMaterialPropertyOld \
  _Pragma( \
      "GCC warning \"adGetMaterialPropertyOld is deprecated. Simply use getMaterialPropertyOld\"") \
  this->template getMaterialPropertyOldTempl
#define adGetMaterialPropertyOlder \
  _Pragma( \
      "GCC warning \"adGetMaterialPropertyOlder is deprecated. Simply use getMaterialPropertyOlder\"") \
  this->template getMaterialPropertyOlderTempl
#define adGetADMaterialProperty \
  _Pragma( \
      "GCC warning \"adGetADMaterialProperty is deprecated. Simply use getADMaterialProperty\"") \
  this->template getADMaterialPropertyTempl
#define adGetADMaterialPropertyByName \
  _Pragma( \
      "GCC warning \"adGetADMaterialPropertyByName is deprecated. Simply use getADMaterialPropertyByName\"") \
  this->template getADMaterialPropertyByNameTempl
#define adGetMaterialPropertyByName \
  _Pragma( \
      "GCC warning \"adGetMaterialPropertyByName is deprecated. Simply use getMaterialPropertyByName\"") \
  this->template getMaterialPropertyByNameTempl
#define adGetMaterialPropertyOldByName \
  _Pragma( \
      "GCC warning \"adGetMaterialPropertyOldByName is deprecated. Simply use getMaterialPropertyOldByName\"") \
  this->template getMaterialPropertyOldByNameTempl
#define adGetMaterialPropertyOlderByName \
  _Pragma( \
      "GCC warning \"adGetMaterialPropertyOlderByName is deprecated. Simply use getMaterialPropertyOlderByName\"") \
  this->template getMaterialPropertyOlderByNameTempl
#define adHasMaterialProperty \
  _Pragma( \
      "GCC warning \"adHasMaterialProperty is deprecated. Simply use hasMaterialProperty\"") \
  this->template hasMaterialPropertyTempl
#define adHasMaterialPropertyByName \
  _Pragma( \
      "GCC warning \"adHasMaterialPropertyByName is deprecated. Simply use hasMaterialPropertyByName\"") \
  this->template hasMaterialPropertyByNameTempl
// clang-format on
#define adDeclareADProperty                                                                        \
  _Pragma("GCC warning \"adDeclareADProperty is deprecated. Simply use declareADProperty\"") this  \
      ->template declareADPropertyTempl
#define adDeclareProperty                                                                          \
  _Pragma("GCC warning \"adDeclareProperty is deprecated. Simply use declareProperty\"") this      \
      ->template declarePropertyTempl

#define getMaterialProperty this->template getMaterialPropertyTempl
#define getMaterialPropertyOld this->template getMaterialPropertyOldTempl
#define getMaterialPropertyOlder this->template getMaterialPropertyOlderTempl
#define declareADProperty this->template declareADPropertyTempl
#define declareProperty this->template declarePropertyTempl
#define getADMaterialProperty this->template getADMaterialPropertyTempl
#define getADMaterialPropertyByName this->template getADMaterialPropertyByNameTempl
#define getMaterialPropertyByName this->template getMaterialPropertyByNameTempl
#define getMaterialPropertyOldByName this->template getMaterialPropertyOldByNameTempl
#define getMaterialPropertyOlderByName this->template getMaterialPropertyOlderByNameTempl
#define hasMaterialProperty this->template hasMaterialPropertyTempl
#define hasMaterialPropertyByName this->template hasMaterialPropertyByNameTempl

// TwoMaterialPropertyInterface

// clang-format off
#define adGetNeighborMaterialProperty \
  _Pragma( \
    "GCC warning \"adGetNeighborMaterialProperty is deprecated. Simply use getNeighborMaterialProperty\"") \
  this->template getNeighborMaterialPropertyTempl
#define adGetNeighborADMaterialProperty \
  _Pragma( \
    "GCC warning \"adGetNeighborADMaterialProperty is deprecated. Simply use getNeighborADMaterialProperty\"") \
  this->template getNeighborADMaterialPropertyTempl
// clang-format on

#define getNeighborMaterialProperty this->template getNeighborMaterialPropertyTempl
#define getNeighborADMaterialProperty this->template getNeighborADMaterialPropertyTempl

// Restartable

// clang-format off
#define adDeclareRestartableData \
  _Pragma( \
    "GCC warning \"adDeclareRestartableData is deprecated. Simply use declareRestartableData\"") \
  this->template declareRestartableDataTempl
// clang-format on

#define declareRestartableData this->template declareRestartableDataTempl
