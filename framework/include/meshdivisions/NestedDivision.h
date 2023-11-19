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
 * Divides the mesh based on nested divisions
 */
class NestedDivision : public MeshDivision
{
public:
  static InputParameters validParams();

  NestedDivision(const InputParameters & parameters);

  virtual void initialize() override;
  virtual unsigned int divisionIndex(const Point & pt) const override;
  virtual unsigned int divisionIndex(const Elem & elem) const override;

protected:
  /// Vector of nested divisions. Indexing is more and more 'inner' as we progress in the vector
  std::vector<const MeshDivision *> _divisions;
  /// Vector of the number of divisions for each nested division
  std::vector<unsigned int> _num_divs;
};
