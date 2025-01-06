#ifdef MFEM_ENABLED

#include "MFEMDataCollection.h"

InputParameters
MFEMDataCollection::validParams()
{
  InputParameters params = FileOutput::validParams();
  params.addClassDescription("Output for controlling MFEMDataCollection inherited data.");
  return params;
}

MFEMDataCollection::MFEMDataCollection(const InputParameters & parameters)
  : FileOutput(parameters),
    _problem_data(static_cast<MFEMProblem *>(_problem_ptr)->getProblemData())
{
}

void
MFEMDataCollection::registerFields()
{
  mfem::DataCollection & dc(getDataCollection());
  for (auto const & [gf_name, gf_ptr] : _problem_data._gridfunctions)
  {
    dc.RegisterField(gf_name, gf_ptr.get());
  }
}

void
MFEMDataCollection::output()
{
  mfem::DataCollection & dc(getDataCollection());
  // Write fields to disk
  dc.SetCycle(getFileNumber());
  dc.SetTime(time());
  dc.Save();
  _file_num++;
}

#endif
