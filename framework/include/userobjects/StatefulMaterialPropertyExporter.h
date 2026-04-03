//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"

/**
 * Exports stateful material property data along with quadrature point
 * positions and subdomain information to a binary file (.smatprop).
 * This data can be loaded by StatefulMaterialPropertyImporter on a
 * different mesh to remap stateful properties using closest-point matching.
 */
class StatefulMaterialPropertyExporter : public GeneralUserObject
{
public:
  static InputParameters validParams();

  StatefulMaterialPropertyExporter(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}

protected:
  /// The file base name for output
  const std::string & _file_base;
};
