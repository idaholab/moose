//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMDataCollection.h"
#include "MFEMEigenproblem.h"

InputParameters
MFEMDataCollection::validParams()
{
  InputParameters params = FileOutput::validParams();
  params.addClassDescription("Output for controlling MFEMDataCollection inherited data.");
  params.addParam<std::string>("submesh",
                               "Submesh to output variables on. Leave blank to use base mesh.");
  return params;
}

MFEMDataCollection::MFEMDataCollection(const InputParameters & parameters)
  : FileOutput(parameters),
    _problem_data(static_cast<MFEMProblem *>(_problem_ptr)->getProblemData()),
    _pmesh(parameters.isParamValid("submesh")
               ? _problem_data.submeshes.GetRef(getParam<std::string>("submesh"))
               : const_cast<mfem::ParMesh &>(*_problem_data.pmesh.get()))
{
}

void
MFEMDataCollection::registerFields()
{
  // Save real fields
  mfem::DataCollection & dc(getDataCollection());
  // For eigenproblems, the bare trial variable holds only initial-guess / essential-BC values
  // rather than a mode solution, so skip it; modes are stored under suffixed names.
  static const std::vector<std::string> empty_names;
  const std::vector<std::string> & skip_names = dynamic_cast<MFEMEigenproblem *>(_problem_ptr)
                                                    ? _problem_data.eqn_system->GetTrialVarNames()
                                                    : empty_names;

  for (auto const & [gf_name, gf_ptr] : _problem_data.gridfunctions)
  {
    if (std::find(skip_names.begin(), skip_names.end(), gf_name) != skip_names.end())
      continue;
    if (dc.GetMesh() == gf_ptr->FESpace()->GetMesh())
      dc.RegisterField(gf_name, gf_ptr.get());
    else
      mooseInfo("The variable ",
                gf_name,
                " is not defined on the same mesh as the output DataCollection.");
  }

  // Save complex fields
  for (auto const & [gf_name, gf_ptr] : _problem_data.cmplx_gridfunctions)
  {
    if (dc.GetMesh() == gf_ptr->FESpace()->GetMesh())
    {
      dc.RegisterField(gf_name + "_real", &gf_ptr->real());
      dc.RegisterField(gf_name + "_imag", &gf_ptr->imag());
    }
    else
      mooseInfo("The variable ",
                gf_name,
                " is not defined on the same mesh as the output DataCollection.");
  }
}

void
MFEMDataCollection::setFileBaseInternal(const std::string & file_base)
{
  FileOutput::setFileBaseInternal(file_base);
  getDataCollection().SetPrefixPath(_file_base);
}

void
MFEMDataCollection::output()
{
  mfem::DataCollection & dc(getDataCollection());
  // Write fields to disk
  dc.SetCycle(getFileNumber());
  dc.SetTime(time());
  dc.Save();
  ++_file_num;
}

#endif
