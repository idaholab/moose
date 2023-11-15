//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshDivision.h"

/**
 * Divides the mesh based on the extra element IDs
 */
class ExtraElementIntegerDivision : public MeshDivision
{
public:
  static InputParameters validParams();

  ExtraElementIntegerDivision(const InputParameters & parameters);

  virtual void initialize() override;
  virtual unsigned int divisionIndex(const Point & pt) const override;
  virtual unsigned int divisionIndex(const Elem & elem) const override;

private:
  /// Map from extra element ids to division index. Created on calls to initialize()
  std::unordered_map<unsigned int, unsigned int> _extra_ids_to_division_index;
  /// Name of the extra element id to use to subdivide
  const ExtraElementIDName & _extra_id_name;
  /// Extra element ID used for the subdivision
  unsigned int _extra_id;
};
