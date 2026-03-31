//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MultiAppMFEMCopyTransfer.h"
#include "FEProblemBase.h"
#include "MultiApp.h"
#include "SystemBase.h"
#include "MFEMProblem.h"
#include "MFEMMesh.h"

registerMooseObject("MooseApp", MultiAppMFEMCopyTransfer);

InputParameters
MultiAppMFEMCopyTransfer::validParams()
{
  InputParameters params = MultiAppTransfer::validParams();
  params.addRequiredParam<std::vector<AuxVariableName>>(
      "variable", "AuxVariable to store transferred value in.");
  params.addRequiredParam<std::vector<VariableName>>("source_variable",
                                                     "Variable to transfer from");
  MooseEnum comp_opts("real imag", "real");
  params.addParam<MooseEnum>("to_component", comp_opts,
                             "Choose whether to copy into the real or imaginary part of a complex target");
  params.addParam<MooseEnum>("from_component", comp_opts,
                             "Choose whether to copy from the real or imaginary part of a complex source");
  params.addClassDescription("Copies variable values from one MFEM application to another");
  return params;
}

MultiAppMFEMCopyTransfer::MultiAppMFEMCopyTransfer(InputParameters const & params)
  : MultiAppTransfer(params),
    _from_var_names(getParam<std::vector<VariableName>>("source_variable")),
    _to_var_names(getParam<std::vector<AuxVariableName>>("variable"))
{
  _to_imag = static_cast<int>(getParam<MooseEnum>("to_component")) == 1;
  _from_imag = static_cast<int>(getParam<MooseEnum>("from_component")) == 1;
  auto bad_problem = [this]()
  {
    mooseError(type(),
               " only works with MFEMProblem based applications. Check that all your inputs "
               "involved in this transfer are MFEMProblem based");
  };
  if (hasToMultiApp())
  {
    if (!dynamic_cast<MFEMProblem *>(&getToMultiApp()->problemBase()))
      bad_problem();
    for (const auto i : make_range(getToMultiApp()->numGlobalApps()))
      if (getToMultiApp()->hasLocalApp(i) &&
          !dynamic_cast<MFEMProblem *>(&getToMultiApp()->appProblemBase(i)))
        bad_problem();
  }
  if (hasFromMultiApp())
  {
    if (!dynamic_cast<MFEMProblem *>(&getFromMultiApp()->problemBase()))
      bad_problem();
    for (const auto i : make_range(getFromMultiApp()->numGlobalApps()))
      if (getFromMultiApp()->hasLocalApp(i) &&
          !dynamic_cast<MFEMProblem *>(&getFromMultiApp()->appProblemBase(i)))
        bad_problem();
  }
}

void
MultiAppMFEMCopyTransfer::transfer(MFEMProblem & to_problem, MFEMProblem & from_problem)
{
  if (numToVar() != numFromVar())
    mooseError("Number of variables transferred must be same in both systems.");

  for (const auto v : make_range(numToVar()))
  {
    const std::string from_name = getFromVarName(v);
    const std::string to_name = getToVarName(v);

    // Detect types in source and target problems
    bool from_is_real = from_problem.getProblemData().gridfunctions.Has(from_name);
    bool from_is_cmplx = from_problem.getProblemData().cmplx_gridfunctions.Has(from_name);
    bool to_is_real = to_problem.getProblemData().gridfunctions.Has(to_name);
    bool to_is_cmplx = to_problem.getProblemData().cmplx_gridfunctions.Has(to_name);

    if (from_is_real && to_is_real)
    {
      mfem::Vector & from_var = *from_problem.getProblemData().gridfunctions.Get(from_name);
      mfem::Vector & to_var = *to_problem.getProblemData().gridfunctions.Get(to_name);
      if (from_var.Size() != to_var.Size())
        mooseError("'", from_name, "' and '", to_name, "' differ in no. of DoFs.");
      to_var = from_var;
    }
    else if (from_is_cmplx && to_is_cmplx)
    {
      mfem::Vector & from_var = *from_problem.getProblemData().cmplx_gridfunctions.Get(from_name);
      mfem::Vector & to_var = *to_problem.getProblemData().cmplx_gridfunctions.Get(to_name);
      if (from_var.Size() != to_var.Size())
        mooseError("'", from_name, "' and '", to_name, "' differ in no. of DoFs.");
      to_var = from_var;
    }
    else if (from_is_real && to_is_cmplx)
    {
      // Copy real source into chosen component of complex target
      mfem::ParGridFunction & from_pf = *from_problem.getProblemData().gridfunctions.Get(from_name);
      mfem::ParComplexGridFunction & to_cpf = *to_problem.getProblemData().cmplx_gridfunctions.Get(to_name);
      mfem::ParGridFunction & target_component = _to_imag ? to_cpf.imag() : to_cpf.real();
      if (from_pf.Size() != target_component.Size())
        mooseError("'", from_name, "' and '", to_name, "' differ in no. of DoFs.");
      target_component = from_pf;
    }
    else if (from_is_cmplx && to_is_real)
    {
      // Copy complex source's selected component into real target
      mfem::ParComplexGridFunction & from_cpf = *from_problem.getProblemData().cmplx_gridfunctions.Get(from_name);
      mfem::ParGridFunction & to_pf = *to_problem.getProblemData().gridfunctions.Get(to_name);
      mfem::ParGridFunction & source_component = _from_imag ? from_cpf.imag() : from_cpf.real();
      if (source_component.Size() != to_pf.Size())
        mooseError("'", from_name, "' and '", to_name, "' differ in no. of DoFs.");
      to_pf = source_component;
    }
    else
    {
      mooseError("No real or complex variable named '", from_name, "' or '", to_name,
                 "' found or unsupported transfer combination.");
    }
  }
}

void
MultiAppMFEMCopyTransfer::execute()
{
  TIME_SECTION("MultiAppMFEMCopyTransfer::execute", 5, "Copies variables");
  if (_current_direction == TO_MULTIAPP)
  {
    for (unsigned int i = 0; i < getToMultiApp()->numGlobalApps(); i++)
    {
      if (getToMultiApp()->hasLocalApp(i))
      {
        transfer(static_cast<MFEMProblem &>(getToMultiApp()->appProblemBase(i)),
                 static_cast<MFEMProblem &>(getToMultiApp()->problemBase()));
      }
    }
  }
  else if (_current_direction == FROM_MULTIAPP)
  {
    for (unsigned int i = 0; i < getFromMultiApp()->numGlobalApps(); i++)
    {
      if (getFromMultiApp()->hasLocalApp(i))
      {
        transfer(static_cast<MFEMProblem &>(getFromMultiApp()->problemBase()),
                 static_cast<MFEMProblem &>(getFromMultiApp()->appProblemBase(i)));
      }
    }
  }
  else if (_current_direction == BETWEEN_MULTIAPP)
  {
    int transfers_done = 0;
    for (unsigned int i = 0; i < getFromMultiApp()->numGlobalApps(); i++)
    {
      if (getFromMultiApp()->hasLocalApp(i))
      {
        if (getToMultiApp()->hasLocalApp(i))
        {
          transfer(static_cast<MFEMProblem &>(getToMultiApp()->appProblemBase(i)),
                   static_cast<MFEMProblem &>(getFromMultiApp()->appProblemBase(i)));
          transfers_done++;
        }
      }
    }
    if (!transfers_done)
      mooseError("BETWEEN_MULTIAPP transfer not supported if there is not at least one subapp "
                 "per multiapp involved on each rank");
  }
}

void
MultiAppMFEMCopyTransfer::checkSiblingsTransferSupported() const
{
  // Check that we are in the supported configuration: same number of source and target apps
  // The allocation of the child apps on the processors must be the same
  if (getFromMultiApp()->numGlobalApps() == getToMultiApp()->numGlobalApps())
  {
    for (const auto i : make_range(getToMultiApp()->numGlobalApps()))
      if (getFromMultiApp()->hasLocalApp(i) + getToMultiApp()->hasLocalApp(i) == 1)
        mooseError("Child application allocation on parallel processes must be the same to support "
                   "siblings variable field copy transfer");
  }
  else
    mooseError("Number of source and target child apps must match for siblings transfer");
}

#endif
