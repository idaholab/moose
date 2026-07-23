//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SBMElementSubdomainModifierBase.h"
#include "Function.h"
#include "PointInPolyhedronCheckUO.h"

enum class DistanceType
{
  NONE = -1,
  SIGN_DISTANCE = 0,
  GEOMETRY = 1
};

class InterceptedElementModifier : public SBMElementSubdomainModifierBase
{
public:
  static InputParameters validParams();
  InterceptedElementModifier(const InputParameters & parameters);

protected:
  virtual SubdomainID computeSubdomainID() override;
  virtual void initialSetup() override;

  /// Store the parsed function
  const Function * _parsed_function;

private:
  /// IDs for subdomain classification (inside)
  SubdomainID _subdomain_id_inside;
  /// IDs for subdomain classification (outside)
  SubdomainID _subdomain_id_outside;

  /// Threshold value for classification
  Real _threshold;

  /// Outer boundary handling flag
  bool _outer_boundary;

  /// user object for in-out test
  const PointInPolyhedronCheckUO * _in_out_test_base;

private:
  /// How to classify the element to be inside or outside
  /// 1. SIGN_DISTANCE: use the signed distance function to classify the element
  /// 2. GEOMETRY: use the geometry to classify the element
  DistanceType _in_out_test_type = DistanceType::NONE;
};
