//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "ConcentricCircleGeneratorBase.h"

/**
 * This AdvancedConcentricCircleGenerator object is designed to mesh a concentric circular geometry.
 */
class AdvancedConcentricCircleGenerator : public ConcentricCircleGeneratorBase
{
public:
  static InputParameters validParams();

  AdvancedConcentricCircleGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// List of the azimuthal angles of the nodes
  std::vector<Real> _azimuthal_angles;
  /// Number of azimuthal sectors of the circular mesh to be generated
  const unsigned int _num_sectors;
  /// A virtual sector number list which are 360.0 times inverse of the azimuthal intervals
  std::vector<Real> _virtual_nums_sectors;
};
