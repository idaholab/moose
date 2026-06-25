//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_LIBTORCH_ENABLED

// libmesh includes
#include "libmesh/petsc_vector.h"

// MOOSE includes
#include "TorchFEInterpolation.h"
#include "TorchFEMUtils.h"

using namespace libMesh;

registerMooseObject("MooseApp", TorchFEInterpolation);

InputParameters
TorchFEInterpolation::validParams()
{
  InputParameters params = ElementUserObject::validParams();

  params.addClassDescription(
      "This user object provides an interface to libtorch for finite element "
      "interpolation of variables and their gradients. It gathers the shape "
      "functions and DOF maps for each variable in the assembly and provides "
      "them as libtorch tensors for use in batched finite-element kernels.");

  params.addRequiredParam<UserObjectName>(
      "assembly", "The TorchAssembly object to use to provide assembly information");

  ExecFlagEnum execute_options = MooseUtils::getDefaultExecFlagEnum();
  execute_options = {EXEC_INITIAL, EXEC_LINEAR};
  params.set<ExecFlagEnum>("execute_on") = execute_options;
  params.suppressParameter<ExecFlagEnum>("execute_on");

  return params;
}

TorchFEInterpolation::TorchFEInterpolation(const InputParameters & parameters)
  : ElementUserObject(parameters), _assembly(getUserObject<TorchAssembly>("assembly"))
{
}

const at::Tensor &
TorchFEInterpolation::getValue(const std::string & var_name)
{
  auto [it, success] = _vars.emplace(var_name, at::Tensor());

  if (success)
  {
    const auto * var = getMOOSEVariable(var_name);
    _phis.emplace(var->feType(), &var->phi());
    _moose_vars.emplace(var_name, var);
  }

  return it->second;
}

const at::Tensor &
TorchFEInterpolation::getGradient(const std::string & var_name)
{
  auto [it, success] = _grad_vars.emplace(var_name, at::Tensor());

  if (success)
  {
    const auto * var = getMOOSEVariable(var_name);
    _grad_phis.emplace(var->feType(), &var->gradPhi());
    _moose_vars.emplace(var_name, var);
  }

  return it->second;
}

const at::Tensor &
TorchFEInterpolation::getPhi(const std::string & var_name)
{
  const auto * var = getMOOSEVariable(var_name);

  const auto it = _torch_phi.find(var->feType());
  if (it != _torch_phi.end())
    return it->second;

  _phis.emplace(var->feType(), &var->phi());
  auto [it2, success] = _torch_phi.emplace(var->feType(), at::Tensor());
  return it2->second;
}

const at::Tensor &
TorchFEInterpolation::getPhiGradient(const std::string & var_name)
{
  const auto * var = getMOOSEVariable(var_name);

  const auto it = _torch_grad_phi.find(var->feType());
  if (it != _torch_grad_phi.end())
    return it->second;

  _grad_phis.emplace(var->feType(), &var->gradPhi());
  auto [it2, success] = _torch_grad_phi.emplace(var->feType(), at::Tensor());
  return it2->second;
}

const at::Tensor &
TorchFEInterpolation::getDofMap(const std::string & var_name)
{
  return _torch_dof_map[var_name];
}

const std::vector<dof_id_type> &
TorchFEInterpolation::getGlobalDofMap(const std::string & var_name)
{
  return _moose_dof_map_global[var_name];
}

int64_t
TorchFEInterpolation::local_ndof() const
{
  return _local_ndof;
}

const MooseVariableFE<Real> *
TorchFEInterpolation::getMOOSEVariable(const std::string & var_name) const
{
  const auto * var = &_fe_problem.getVariable(
      0, var_name, Moose::VarKindType::VAR_SOLVER, Moose::VarFieldType::VAR_FIELD_STANDARD);
  const auto * var_fe = dynamic_cast<const MooseVariableFE<Real> *>(var);

  if (!var_fe)
    mooseError("TorchFEInterpolation only supports variables of type MooseVariableFE<Real>");

  if (var_fe->scalingFactor() != 1)
    mooseError("Scaling factors other than unity are not yet supported");

  // check domain restrictions for compatibility
  if (!var_fe->hasBlocks(blockIDs()))
    mooseError("The variable '",
               var_fe->name(),
               "' must be defined on all blocks '",
               name(),
               "' is defined on.");

  return var_fe;
}

void
TorchFEInterpolation::initialSetup()
{
  _petsc_solution = dynamic_cast<const PetscVector<Real> *>(_sys.currentSolution());

  // check if the solution vector is of a supported type
  if (!_petsc_solution)
    mooseError("Only solution vectors of type PetscVector are currently supported");

  if (_tid != 0)
    syncWithMainThread();
}

void
TorchFEInterpolation::invalidateFEMContext()
{
  _fem_context_up_to_date = false;
  invalidateInterpolations();
}

void
TorchFEInterpolation::invalidateInterpolations()
{
  _interp_up_to_date = false;
}

void
TorchFEInterpolation::meshChanged()
{
  invalidateFEMContext();
}

void
TorchFEInterpolation::initialize()
{
  if (_fem_context_up_to_date)
    return;

  _ndofe.clear();
  _moose_dof_map.clear();
  _moose_dof_map_global.clear();
  _moose_phi.clear();
  _moose_grad_phi.clear();
  _local_ndof = 0;
}

void
TorchFEInterpolation::syncWithMainThread()
{
  auto & main_uo = _fe_problem.getUserObject<TorchFEInterpolation>(name(), /*tid=*/0);
  for (const auto & [var_name, var] : main_uo._moose_vars)
  {
    _moose_vars.emplace(var_name, getMOOSEVariable(var_name));
    if (main_uo._phis.count(var->feType()))
      getPhi(var_name);
    if (main_uo._grad_phis.count(var->feType()))
      getPhiGradient(var_name);
  }
}

void
TorchFEInterpolation::threadJoin(const UserObject & y)
{
  const auto & other = static_cast<const TorchFEInterpolation &>(y);
  mooseAssert(_fem_context_up_to_date == other._fem_context_up_to_date,
              "TorchFEInterpolation becomes out of sync with other thread");

  if (_fem_context_up_to_date)
    return;

  auto merge_map_vecs = [](auto & map1, const auto & map2)
  {
    for (const auto & [key, map2_val] : map2)
    {
      auto & map1_val = map1[key];
      map1_val.insert(map1_val.end(), map2_val.begin(), map2_val.end());
    }
  };

  merge_map_vecs(_moose_dof_map, other._moose_dof_map);
  merge_map_vecs(_moose_phi, other._moose_phi);
  merge_map_vecs(_moose_grad_phi, other._moose_grad_phi);
}

void
TorchFEInterpolation::execute()
{
  if (_fem_context_up_to_date)
    return;

  // DOF indices
  const auto & nl_dof_map = _sys.dofMap();
  for (const auto & [var_name, var] : _moose_vars)
  {
    nl_dof_map.dof_indices(_current_elem, _dof_indices, var->number());
    auto [it, success] = _ndofe.emplace(var->feType(), _dof_indices.size());
    if (!success && std::size_t(it->second) != _dof_indices.size())
      mooseError("DOF map size mismatch for variable ",
                 var_name,
                 ", got ",
                 it->second,
                 " and ",
                 _dof_indices.size());
    auto & moose_dof_map = _moose_dof_map[var_name];
    auto & moose_dof_map_global = _moose_dof_map_global[var_name];
    for (auto dof : _dof_indices)
    {
      moose_dof_map.push_back(_petsc_solution->map_global_to_local_index(dof));
      moose_dof_map_global.push_back(dof);
    }
  }

  // shape function values
  for (const auto & [fetype, phi] : _phis)
  {
    auto & moose_phi = _moose_phi[fetype];
    for (auto i : index_range(*phi))
      for (auto qp : index_range(_q_point))
        moose_phi.push_back((*phi)[i][qp]);
  }

  // shape function gradients
  for (const auto & [fetype, grad_phi] : _grad_phis)
  {
    auto & moose_grad_phi = _moose_grad_phi[fetype];
    for (auto i : index_range(*grad_phi))
      for (auto qp : index_range(_q_point))
        for (auto j : make_range(3))
          moose_grad_phi.push_back((*grad_phi)[i][qp](j));
  }
}

void
TorchFEInterpolation::finalize()
{
  TIME_SECTION("finalize", 1, "Updating FEM context and interpolations for the Torch FEM kernels");

  if (!_fem_context_up_to_date)
    updateFEMContext();

  if (!_interp_up_to_date)
    updateInterpolations();
}

void
TorchFEInterpolation::updateFEMContext()
{
  TIME_SECTION("updateFEMContext", 2, "Updating FEM context for the Torch FEM kernels");

  updateDofMap();
  updatePhi();
  updateGradPhi();

  // done
  _fem_context_up_to_date = true;
}

void
TorchFEInterpolation::updateDofMap()
{
  auto device = _app.getLibtorchDevice();
  auto nelem = _assembly.numElem();

  for (auto & [var_name, moose_dof_map] : _moose_dof_map)
  {
    auto var = _moose_vars.at(var_name);
    auto ndofe = _ndofe.at(var->feType());

    // sanity check on sizes
    if (moose_dof_map.size() != std::size_t(nelem * ndofe))
      mooseError(
          "dof map size mismatch, expected ", nelem * ndofe, " but got ", moose_dof_map.size());

    // convert to a libtorch tensor
    _torch_dof_map[var_name] =
        at::from_blob(moose_dof_map.data(), {nelem, ndofe}, torch::kInt64).to(device);

    _local_ndof = std::max(_local_ndof, _torch_dof_map[var_name].max().item<int64_t>() + 1);
  }
}

void
TorchFEInterpolation::updatePhi()
{
  auto device = _app.getLibtorchDevice();
  auto nelem = _assembly.numElem();
  auto nqp = _assembly.numQP();

  for (auto & [fetype, moose_phi] : _moose_phi)
  {
    auto ndofe = _ndofe.at(fetype);

    // sanity check on sizes
    if (moose_phi.size() != std::size_t(nelem * ndofe * nqp))
      mooseError("shape function size mismatch, expected ",
                 nelem * ndofe * nqp,
                 " but got ",
                 moose_phi.size());
    _torch_phi[fetype] =
        at::from_blob(moose_phi.data(), {nelem, ndofe, nqp}, torch::kFloat64).to(device);
  }
}

void
TorchFEInterpolation::updateGradPhi()
{
  auto device = _app.getLibtorchDevice();
  auto nelem = _assembly.numElem();
  auto nqp = _assembly.numQP();

  for (auto & [fetype, moose_grad_phi] : _moose_grad_phi)
  {
    auto ndofe = _ndofe.at(fetype);

    // sanity check on sizes
    if (moose_grad_phi.size() != std::size_t(nelem * ndofe * nqp * 3))
      mooseError("shape function gradient size mismatch, expected ",
                 nelem * ndofe * nqp * 3,
                 " but got ",
                 moose_grad_phi.size());
    _torch_grad_phi[fetype] =
        at::from_blob(moose_grad_phi.data(), {nelem, ndofe, nqp, 3}, torch::kFloat64).to(device);
  }
}

void
TorchFEInterpolation::updateInterpolations()
{
  TIME_SECTION("updateInterpolations", 2, "Updating FEM interpolations for the Torch FEM kernels");

  // convert the local solution vector to a libtorch tensor
  auto sol = at::from_blob(const_cast<Real *>(_petsc_solution->get_array_read()),
                           {local_ndof()},
                           torch::kFloat64)
                 .to(_app.getLibtorchDevice());

  // interpolate variable values
  for (auto & [var_name, val] : _vars)
  {
    const auto & dof_map = _torch_dof_map[var_name];
    const auto fetype = _moose_vars[var_name]->feType();
    const auto & phi = _torch_phi[fetype];
    auto sol_scattered = TorchFEM::scatter(sol, dof_map);
    val = TorchFEM::interpolate(sol_scattered, phi);
  }

  // interpolate variable gradients
  for (auto & [var_name, val] : _grad_vars)
  {
    const auto & dof_map = _torch_dof_map[var_name];
    const auto fetype = _moose_vars[var_name]->feType();
    const auto & grad_phi = _torch_grad_phi[fetype];
    auto sol_scattered = TorchFEM::scatter(sol, dof_map);
    val = TorchFEM::interpolate(sol_scattered, grad_phi);
  }

  // close solution and residual vector access
  const_cast<PetscVector<Real> *>(_petsc_solution)->restore_array();

  // done
  _interp_up_to_date = true;
}

#endif // MOOSE_LIBTORCH_ENABLED
