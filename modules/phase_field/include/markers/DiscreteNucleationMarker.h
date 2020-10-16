//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Marker.h"
#include "DiscreteNucleationInserterBase.h"

class DiscreteNucleationMap;

/**
 * Mark new nucleation sites for refinement
 */
class DiscreteNucleationMarker : public Marker
{
public:
  static InputParameters validParams();

  DiscreteNucleationMarker(const InputParameters & parameters);

protected:
  virtual MarkerValue computeElementMarker() override;

  /// UserObject providing a map of currently active nuclei
  const DiscreteNucleationMap & _map;

  /// variable number to use for minPeriodicDistance calls (i.e. use the periodicity of this variable)
  int _periodic;

  /// UserObject that manages nucleus insertin and deletion
  const DiscreteNucleationInserterBase & _inserter;

  /// Nucleus interface width
  const Real _int_width;

  /// list of nuclei maintained bu the inserter object
  const DiscreteNucleationInserterBase::NucleusList & _nucleus_list;
};
