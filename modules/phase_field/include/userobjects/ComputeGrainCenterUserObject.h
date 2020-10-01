//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementUserObject.h"

// Forward Declarations

/**
 * This UserObject computes a volumes and centers of grains.
 */
class ComputeGrainCenterUserObject : public ElementUserObject
{
public:
  static InputParameters validParams();

  ComputeGrainCenterUserObject(const InputParameters & parameters);

  virtual void initialize();
  virtual void execute();
  virtual void threadJoin(const UserObject & y);
  virtual void finalize();

  const std::vector<Real> & getGrainVolumes() const;
  const std::vector<Point> & getGrainCenters() const;

protected:
  unsigned int _qp;
  unsigned int _ncrys;
  const std::vector<const VariableValue *> _vals;
  unsigned int _ncomp;
  ///@{ storing volumes and centers of all the grains
  std::vector<Real> _grain_data;
  std::vector<Real> _grain_volumes;
  std::vector<Point> _grain_centers;
  ///@}
};
