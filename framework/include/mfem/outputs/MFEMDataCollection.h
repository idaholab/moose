#ifdef MFEM_ENABLED

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

  /// Reference to the MFEMProblemData struct storing the output variables.
  MFEMProblemData & _problem_data;
  /// Mesh set of output variables are defined on. May differ from main problem mesh if SubMesh
  /// variables are in use.
  mfem::ParMesh & _pmesh;
};

#endif
