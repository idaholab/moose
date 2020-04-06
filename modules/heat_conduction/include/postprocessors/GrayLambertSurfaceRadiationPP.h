//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"
#include "GrayLambertSurfaceRadiationBase.h"

/**
 * A postprocessor that extracts information from
 * the GrayLambertSurfaceRadiationBase UserObject
 */
class GrayLambertSurfaceRadiationPP : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  GrayLambertSurfaceRadiationPP(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual PostprocessorValue getValue() override;

protected:
  const GrayLambertSurfaceRadiationBase & _glsr_uo;
  MooseEnum _return_type;
  BoundaryID _bnd_id;
};
