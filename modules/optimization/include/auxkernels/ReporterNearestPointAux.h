//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"
#include "ReporterInterface.h"

/**
 &ant auxiliary value
 */
class ReporterNearestPointAux : public AuxKernel, public ReporterInterface
{
public:
  static InputParameters validParams();

  ReporterNearestPointAux(const InputParameters & parameters);

  virtual void subdomainSetup() override;

protected:
  virtual Real computeValue() override;

  /// x-coordinates from reporter
  const std::vector<Real> & _coordx;
  /// y-coordinates from reporter
  const std::vector<Real> & _coordy;
  /// z-coordinates from reporter
  const std::vector<Real> & _coordz;
  /// time-coordinates from reporter
  const std::vector<Real> & _coordt;
  /// values from reporter
  const std::vector<Real> & _values;

  /// Data for the current time
  std::unordered_map<Point, Real> _current_data;

private:
  const std::vector<Real> _empty_vec = {};
};
