//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "GrainTracker.h"
#include "EBSDReader.h"

/**
 * Visualize the location of grain boundaries in a polycrystalline simulation.
 */
class ComputeGBMisoType : public Material
{
public:
  static InputParameters validParams();

  ComputeGBMisoType(const InputParameters & parameters);

protected:
  /// Necessary override. This is where the property values are set.
  virtual void computeQpProperties() override;

  // Function to read the miso angle form a file
  void readFile();
  // Function to output total line number of miso angle file
  virtual unsigned int getTotalLineNum() const;
  // Function to output specific line number in miso angle file
  virtual unsigned int getLineNum(unsigned int, unsigned int);
  // Function to get the GB type for triple junctions
  virtual Real getTripleJunctionType(std::vector<unsigned int>, std::vector<Real>);

  /// Grain tracker object
  const GrainTracker & _grain_tracker;

  /// EBSD reader user object
  const EBSDReader & _ebsd_reader;

  // Parameters to read the miso angle file
  FileName _file_name;
  std::vector<Real> _miso_angles;

  // order parameters
  const unsigned int _op_num;
  const std::vector<const VariableValue *> _vals;

  // the max value of LAGB
  const Real _angle_max;

  /// precalculated element value
  ADMaterialProperty<Real> & _gb_type;
};
