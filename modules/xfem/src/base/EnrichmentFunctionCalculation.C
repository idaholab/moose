//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EnrichmentFunctionCalculation.h"

EnrichmentFunctionCalculation::EnrichmentFunctionCalculation(
    const CrackFrontDefinition * crack_front_definition)
  : _crack_front_definition(*crack_front_definition)
{
}

unsigned int
EnrichmentFunctionCalculation::crackTipEnrichementFunctionAtPoint(const Point & point,
                                                                  std::vector<Real> & B)
{
  unsigned int crack_front_point_index =
      _crack_front_definition.calculateRThetaToCrackFront(point, _r, _theta);

  if (MooseUtils::absoluteFuzzyEqual(_r, 0.0))
    mooseError("EnrichmentFunctionCalculation: the distance between a point and the crack "
               "tip/front is zero.");

  Real st = std::sin(_theta);
  Real st2 = std::sin(_theta / 2.0);
  Real ct2 = std::cos(_theta / 2.0);
  Real sr = std::sqrt(_r);

  B[0] = sr * st2;
  B[1] = sr * ct2;
  B[2] = sr * st2 * st;
  B[3] = sr * ct2 * st;

  return crack_front_point_index;
}

unsigned int
EnrichmentFunctionCalculation::crackTipEnrichementFunctionDerivativeAtPoint(
    const Point & point, std::vector<RealVectorValue> & dB)
{
  unsigned int crack_front_point_index =
      _crack_front_definition.calculateRThetaToCrackFront(point, _r, _theta);

  if (MooseUtils::absoluteFuzzyEqual(_r, 0.0))
    mooseError("EnrichmentFunctionCalculation: the distance between a point and the crack "
               "tip/front is zero.");

  Real st = std::sin(_theta);
  Real ct = std::cos(_theta);
  Real st2 = std::sin(_theta / 2.0);
  Real ct2 = std::cos(_theta / 2.0);
  Real st15 = std::sin(1.5 * _theta);
  Real ct15 = std::cos(1.5 * _theta);
  Real sr = std::sqrt(_r);

  dB[0](0) = -0.5 / sr * st2;
  dB[0](1) = 0.5 / sr * ct2;
  dB[0](2) = 0.0;
  dB[1](0) = 0.5 / sr * ct2;
  dB[1](1) = 0.5 / sr * st2;
  dB[1](2) = 0.0;
  dB[2](0) = -0.5 / sr * st15 * st;
  dB[2](1) = 0.5 / sr * (st2 + st15 * ct);
  dB[2](2) = 0.0;
  dB[3](0) = -0.5 / sr * ct15 * st;
  dB[3](1) = 0.5 / sr * (ct2 + ct15 * ct);
  dB[3](2) = 0.0;

  return crack_front_point_index;
}

void
EnrichmentFunctionCalculation::rotateFromCrackFrontCoordsToGlobal(const RealVectorValue & vector,
                                                                  RealVectorValue & rotated_vector,
                                                                  const unsigned int point_index)
{
  rotated_vector = _crack_front_definition.rotateFromCrackFrontCoordsToGlobal(vector, point_index);
}
