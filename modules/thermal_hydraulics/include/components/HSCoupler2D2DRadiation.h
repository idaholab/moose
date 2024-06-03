//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "BoundaryBase.h"
#include "MeshAlignment2D2D.h"

/**
 * Couples boundaries of multiple 2D heat structures via radiation
 */
class HSCoupler2D2DRadiation : public BoundaryBase
{
public:
  static InputParameters validParams();

  HSCoupler2D2DRadiation(const InputParameters & parameters);

  virtual void addMooseObjects() override;

protected:
  virtual void setupMesh() override;
  virtual void check() const override;

  /// Heat structure names
  const std::vector<std::string> _hs_names;
  /// Heat structure boundary names
  const std::vector<BoundaryName> _hs_boundaries;
  /// Whether or not to include an environment surrounding all of the surfaces
  const bool _include_environment;

  /// Number of heat structures
  const unsigned int _n_hs;
  /// Number of surfaces
  const unsigned int _n_surfaces;

  /// Mesh alignment object
  MeshAlignment2D2D _mesh_alignment;
};
