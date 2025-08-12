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

/**
 * Class for output information saved in MFEM DataCollections
 */
class MFEMDataCollection : public FileOutput
{
public:
  static InputParameters validParams();
  MFEMDataCollection(const InputParameters & parameters);
  virtual mfem::DataCollection & getDataCollection() = 0;

protected:
  void registerFields();
  void output() override;
  /**
   * Whether the grid functions need to be explicitly read on the host before output. This seems to
   * be required for Conduit data output whereas other formats handle the host read themselves
   */
  virtual bool gridFunctionsNeedHostRead() const { return false; }

  /// Reference to the MFEMProblemData struct storing the output variables.
  MFEMProblemData & _problem_data;
  /// Mesh set of output variables are defined on. May differ from main problem mesh if SubMesh
  /// variables are in use.
  mfem::ParMesh & _pmesh;
};

#endif
