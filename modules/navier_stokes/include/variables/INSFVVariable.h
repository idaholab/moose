//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseVariableFV.h"

class InputParameters;

class INSFVVariable : public MooseVariableFVReal
{
public:
  INSFVVariable(const InputParameters & params);

  static InputParameters validParams();

  // The INSFV system relies on on-the-fly functor evaluation and does not need any pre-init'd data
  void computeFaceValues(const FaceInfo &) override {}
  void computeElemValues() override { _element_data->setGeometry(Moose::Volume); }
  void computeElemValuesFace() override {}
  void computeNeighborValuesFace() override {}
  void computeNeighborValues() override {}

protected:
  /**
   * Returns whether the passed-in \p FaceInfo corresponds to a fully-developed flow face
   */
  bool isFullyDevelopedFlowFace(const FaceInfo & fi) const;
};
