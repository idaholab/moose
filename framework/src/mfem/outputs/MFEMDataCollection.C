#ifdef MFEM_ENABLED

#include "MFEMDataCollection.h"

InputParameters
MFEMDataCollection::validParams()
{
  InputParameters params = FileOutput::validParams();
  params.addClassDescription("Output for controlling MFEMDataCollection inherited data.");
  params.addParam<std::string>(
      "submesh", "", "Submesh to output variables on. Leave blank to use base mesh.");
  return params;
}

MFEMDataCollection::MFEMDataCollection(const InputParameters & parameters)
  : FileOutput(parameters),
    _problem_data(static_cast<MFEMProblem *>(_problem_ptr)->getProblemData()),
    _submesh_name(getParam<std::string>("submesh")),
    _pmesh(parameters.isParamSetByUser("submesh")
               ? _problem_data.submeshes.GetRef(_submesh_name)
               : const_cast<mfem::ParMesh &>(*_problem_data.pmesh.get()))
{
}

void
MFEMDataCollection::registerFields()
{
  mfem::DataCollection & dc(getDataCollection());
  for (auto const & [gf_name, gf_ptr] : _problem_data.gridfunctions)
  {
    if (dc.GetMesh() == gf_ptr->FESpace()->GetMesh())
    {
      dc.RegisterField(gf_name, gf_ptr.get());
    }
    else
    {
      mooseWarning("The variable ",
                   gf_name,
                   " is not defined on the same mesh as the output DataCollection.");
    }
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
