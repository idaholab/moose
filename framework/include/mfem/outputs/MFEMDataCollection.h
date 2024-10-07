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

  MFEMProblemData & _problem_data;
};
