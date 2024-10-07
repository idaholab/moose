#pragma once
#include "FileOutput.h"
#include "MFEMProblem.h"
#include "mfem.hpp"

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
  MFEMProblemData & _problem_data;
  void output() override
  {
    mfem::DataCollection & dc(getDataCollection());
    // Write fields to disk
    dc.SetCycle(getFileNumber());
    dc.SetTime(time());
    dc.Save();
    _file_num++;
  }
};
