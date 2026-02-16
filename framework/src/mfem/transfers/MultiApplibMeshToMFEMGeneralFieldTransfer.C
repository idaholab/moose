//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MultiApplibMeshToMFEMGeneralFieldTransfer.h"
#include "FEProblemBase.h"
#include "MultiApp.h"
#include "SystemBase.h"
#include "MFEMProblem.h"
#include "MFEMMesh.h"
#include "MFEMVectorFromlibMeshPoint.h"

#include "libmesh/mesh_function.h"

registerMooseObject("MooseApp", MultiApplibMeshToMFEMGeneralFieldTransfer);

InputParameters
MultiApplibMeshToMFEMGeneralFieldTransfer::validParams()
{
  InputParameters params = MultiAppTransfer::validParams();
  params.addRequiredParam<std::vector<AuxVariableName>>(
      "variable", "AuxVariable to store transferred value in.");
  params.addRequiredParam<std::vector<VariableName>>("source_variable",
                                                     "Variable to transfer from");
  params.addClassDescription("Copies variable values from MFEM subapp to libMesh.");
  return params;
}

MultiApplibMeshToMFEMGeneralFieldTransfer::MultiApplibMeshToMFEMGeneralFieldTransfer(InputParameters const & params)
  : MultiAppTransfer(params),
    _mfem_interpolator(this->comm().get()),
    _from_var_names(getParam<std::vector<VariableName>>("source_variable")),
    _to_var_names(getParam<std::vector<AuxVariableName>>("variable"))
{
  auto bad_problem = [this]()
  {
    mooseError(type(),
               " only works with MFEMProblem based applications. Check that all your inputs "
               "involved in this transfer are MFEMProblem based");
  };

  if (hasToMultiApp())
  {
    if (!dynamic_cast<FEProblemBase *>(&getToMultiApp()->problemBase()))
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
          !dynamic_cast<FEProblemBase *>(&getFromMultiApp()->appProblemBase(i)))
        bad_problem();
  }
}

void
MultiApplibMeshToMFEMGeneralFieldTransfer::transfer(MFEMProblem & to_problem, FEProblemBase & from_problem)
{
  if (numToVar() != numFromVar())
    mooseError("Number of variables transferred must be same in both systems.");

  for (unsigned v = 0; v < numToVar(); ++v)
    setMFEMGridFunctionValuesFromlibMesh(v, to_problem);
}

void
MultiApplibMeshToMFEMGeneralFieldTransfer::setMFEMGridFunctionValuesFromlibMesh(const unsigned int var_index, MFEMProblem & to_problem)
{
  std::vector<libMesh::MeshFunction> local_meshfuns;
  local_meshfuns.clear();
  local_meshfuns.reserve(_from_problems.size());

  // Construct a local mesh function for each origin problem
  for (unsigned int i_from = 0; i_from < _from_problems.size(); ++i_from)
  {
    FEProblemBase & from_problem = *_from_problems[i_from];
    MooseVariableFieldBase & from_var =
        from_problem.getVariable(0,
                                 _from_var_names[var_index],
                                 Moose::VarKindType::VAR_ANY,
                                 Moose::VarFieldType::VAR_FIELD_ANY);

    System & from_sys = from_var.sys().system();
    unsigned int from_var_num = from_sys.variable_number(getFromVarName(var_index));

    local_meshfuns.emplace_back(from_problem.es(),
                                *from_sys.current_local_solution,
                                from_sys.get_dof_map(),
                                from_var_num);
    local_meshfuns.back().init();
    // local_meshfuns.back().enable_out_of_mesh_mode(GeneralFieldTransfer::BetterOutOfMeshValue);
  }


  auto & to_var = to_problem.getProblemData().gridfunctions.GetRef(getFromVarName(var_index));
  mfem::FunctionCoefficient coef(
      [this, &local_meshfuns](const mfem::Vector & p, mfem::real_t t) -> mfem::real_t
      {
        int i_from = 0;
        const auto from_global_num = getGlobalSourceAppIndex(i_from);
        const auto local_pt = _from_transforms[from_global_num]->mapBack(pointFromMFEMVector(p));
        auto val = (local_meshfuns[i_from])(local_pt);
        return val;
      });  
  to_var.ProjectCoefficient(coef);

}

void
MultiApplibMeshToMFEMGeneralFieldTransfer::execute()
{
  TIME_SECTION("MultiApplibMeshToMFEMGeneralFieldTransfer::execute", 5, "Copies variables");
  if (_current_direction == TO_MULTIAPP)
  {
    for (unsigned int i = 0; i < getToMultiApp()->numGlobalApps(); i++)
    {
      if (getToMultiApp()->hasLocalApp(i))
      {
        transfer(static_cast<MFEMProblem &>(getToMultiApp()->appProblemBase(i)),
                 getToMultiApp()->problemBase());
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
                 getFromMultiApp()->appProblemBase(i));
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
                   getFromMultiApp()->appProblemBase(i));
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
MultiApplibMeshToMFEMGeneralFieldTransfer::checkSiblingsTransferSupported() const
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
