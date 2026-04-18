//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "FileOutput.h"
#include "MFEMProblem.h"

namespace Moose::MFEM
{
/**
 * Class for output information saved in MFEM DataCollections
 */
class DataCollection : public FileOutput
{
public:
  static InputParameters validParams();
  DataCollection(const InputParameters & parameters);
  virtual mfem::DataCollection & getDataCollection() = 0;

protected:
  void registerFields();
  void output() override;

  /// Reference to the ProblemData struct storing the output variables.
  ProblemData & _problem_data;
  /// Mesh set of output variables are defined on. May differ from main problem mesh if SubMesh
  /// variables are in use.
  mfem::ParMesh & _pmesh;
};

} // namespace Moose::MFEM
#endif
