//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// This file is for member template functions that are defined in multiple classes

#define adGetParam                                                                                 \
  _Pragma("GCC warning \"adGetParam is deprecated. Simply use getParam\"") this                    \
      ->template getParamTempl
#define getParam this->template getParamTempl

#define adGetUserObject                                                                            \
  _Pragma("GCC warning \"adGetUserObject is deprecated. Simply use getUserObject\"") this          \
      ->template getUserObjectTempl
#define getUserObject this->template getUserObjectTempl

#define adGetMaterialProperty                                                                      \
  _Pragma(                                                                                         \
      "GCC warning \"adGetMaterialProperty is deprecated. Simply use getMaterialProperty\"") this  \
      ->template getMaterialPropertyTempl
#define getMaterialProperty this->template getMaterialPropertyTempl

// clang-format off
#define adGetMaterialPropertyOld \
  _Pragma( \
      "GCC warning \"adGetMaterialPropertyOld is deprecated. Simply use getMaterialPropertyOld\"") \
  this->template getMaterialPropertyOldTempl
#define adGetMaterialPropertyOlder \
  _Pragma( \
      "GCC warning \"adGetMaterialPropertyOlder is deprecated. Simply use getMaterialPropertyOlder\"") \
  this->template getMaterialPropertyOlderTempl
// clang-format on
#define getMaterialPropertyOld this->template getMaterialPropertyOldTempl
#define getMaterialPropertyOlder this->template getMaterialPropertyOlderTempl

#define adGetUserObjectByName                                                                      \
  _Pragma(                                                                                         \
      "GCC warning \"adGetUserObjectByName is deprecated. Simply use getUserObjectByName\"") this  \
      ->template getUserObjectByNameTempl
#define getUserObjectByName this->template getUserObjectByNameTempl
