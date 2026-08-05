//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/point.h"

/// How the ray-casting engine obtains its shooting direction.
enum class RayDirectionMode
{
  /// The engine auto-selects a robust direction via PCA (may use a fallback).
  AUTO_PCA,
  /// The engine uses the caller-supplied direction exactly, with no auto-selection.
  USER_SPECIFIED
};

/// Ray-direction intent for AdaptiveRayContainmentCheck: an explicit mode plus the
/// direction to use when mode == USER_SPECIFIED (ignored for AUTO_PCA).
struct RayDirectionOptions
{
  RayDirectionMode mode = RayDirectionMode::AUTO_PCA;
  libMesh::Point direction = libMesh::Point(0.0, 0.0, 0.0);
};
