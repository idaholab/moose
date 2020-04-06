//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PolycrystalUserObjectBase.h"

// Forward Declarations
class EBSDReader;

class PolycrystalEBSD : public PolycrystalUserObjectBase
{
public:
  static InputParameters validParams();

  PolycrystalEBSD(const InputParameters & parameters);

  virtual void getGrainsBasedOnPoint(const Point & point,
                                     std::vector<unsigned int> & grains) const override;
  virtual Real getVariableValue(unsigned int op_index, const Point & p) const override;
  virtual Real getNodalVariableValue(unsigned int op_index, const Node & n) const override;
  virtual unsigned int getNumGrains() const override;

protected:
  const unsigned int _phase;
  const EBSDReader & _ebsd_reader;
  const std::map<dof_id_type, std::vector<Real>> & _node_to_grain_weight_map;
};
