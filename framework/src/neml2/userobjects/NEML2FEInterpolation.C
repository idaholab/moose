//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef NEML2_ENABLED

// libmesh includes
#include "libmesh/petsc_vector.h"

// Torch includes
#include <ATen/ops/from_blob.h>

// NEML2 includes
#include "neml2/tensors/functions/discretization/scatter.h"
#include "neml2/tensors/functions/discretization/interpolate.h"

// MOOSE includes
#include "NEML2FEInterpolation.h"

using namespace libMesh;

registerMooseObject("MooseApp", NEML2FEInterpolation);

InputParameters
NEML2FEInterpolation::validParams()
{
  InputParameters params = ElementUserObject::validParams();

  params.addClassDescription(
      "This user object provides an interface to NEML2 for finite element "
      "interpolation of variables and their gradients. It gathers the shape "
      "functions and DOF maps for each variable in the assembly and provides "
      "them as NEML2 tensors for use in NEML2 models.");

  params.addRequiredParam<UserObjectName>(
      "assembly", "The NEML2Assembly object to use to provide assembly information");

  ExecFlagEnum execute_options = MooseUtils::getDefaultExecFlagEnum();
  execute_options = {EXEC_INITIAL, EXEC_LINEAR};
  params.set<ExecFlagEnum>("execute_on") = execute_options;
  params.suppressParameter<ExecFlagEnum>("execute_on");

  return params;
}

NEML2FEInterpolation::NEML2FEInterpolation(const InputParameters & parameters)
  : ElementUserObject(parameters), _neml2_assembly(getUserObject<NEML2Assembly>("assembly"))
{
}

const neml2::Tensor &
NEML2FEInterpolation::getValue(const std::string & var_name) const
{
  auto [it, success] = _vars.emplace(var_name, neml2::Tensor());

  if (success)
  {
    const auto * var = getMOOSEVariable(var_name);
    _phis.emplace(var->feType(), &var->phi());
    _moose_vars.emplace(var_name, var);
  }

  return it->second;
}

const neml2::Tensor &
NEML2FEInterpolation::getGradient(const std::string & var_name) const
{
  auto [it, success] = _grad_vars.emplace(var_name, neml2::Tensor());

  if (success)
  {
    const auto * var = getMOOSEVariable(var_name);
    _grad_phis.emplace(var->feType(), &var->gradPhi());
    _moose_vars.emplace(var_name, var);
  }

  return it->second;
}

const neml2::Tensor &
NEML2FEInterpolation::getPhi(const std::string & var_name) const
{
  const auto * var = getMOOSEVariable(var_name);

  const auto it = _neml2_phi.find(var->feType());
  if (it != _neml2_phi.end())
    return it->second;

  _phis.emplace(var->feType(), &var->phi());
  auto [it2, success] = _neml2_phi.emplace(var->feType(), neml2::Tensor());
  return it2->second;
}

const neml2::Tensor &
NEML2FEInterpolation::getPhiGradient(const std::string & var_name) const
{
  const auto * var = getMOOSEVariable(var_name);

  const auto it = _neml2_grad_phi.find(var->feType());
  if (it != _neml2_grad_phi.end())
    return it->second;

  _grad_phis.emplace(var->feType(), &var->gradPhi());
  auto [it2, success] = _neml2_grad_phi.emplace(var->feType(), neml2::Tensor());
  return it2->second;
}

const neml2::Tensor &
NEML2FEInterpolation::getDofMap(const std::string & var_name) const
{
  return _neml2_dof_map[var_name];
}

const std::vector<dof_id_type> &
NEML2FEInterpolation::getGlobalDofMap(const std::string & var_name) const
{
  return _moose_dof_map_global.at(var_name);
}

int64_t
NEML2FEInterpolation::local_ndof() const
{
  return _local_ndof;
}

const MooseVariableFE<Real> *
NEML2FEInterpolation::getMOOSEVariable(const std::string & var_name) const
{
  const auto * var = &_fe_problem.getVariable(
      0, var_name, Moose::VarKindType::VAR_SOLVER, Moose::VarFieldType::VAR_FIELD_STANDARD);
  const auto * var_fe = dynamic_cast<const MooseVariableFE<Real> *>(var);

  if (!var_fe)
    mooseError("NEML2FEInterpolation only supports variables of type MooseVariableFE<Real>");

  if (var_fe->scalingFactor() != 1)
    mooseError("Scaling factors other than unity are not yet supported");

  return var_fe;
}

void
NEML2FEInterpolation::initialSetup()
{
  _petsc_solution = dynamic_cast<const PetscVector<Real> *>(_sys.currentSolution());

  // check if the solution vector is of a supported type
  if (!_petsc_solution)
    mooseError("Only solution vectors of type PetscVector are currently supported");

  if (_tid != 0)
    syncWithMainThread();
}

void
NEML2FEInterpolation::invalidateFEMContext() const
{
  _fem_context_up_to_date = false;
  invalidateInterpolations();
}

void
NEML2FEInterpolation::invalidateInterpolations() const
{
  _interp_up_to_date = false;
}

void
NEML2FEInterpolation::meshChanged()
{
  invalidateFEMContext();
}

void
NEML2FEInterpolation::initialize()
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
NEML2FEInterpolation::syncWithMainThread()
{
  auto & main_uo = _fe_problem.getUserObject<NEML2FEInterpolation>(name(), /*tid=*/0);
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
NEML2FEInterpolation::threadJoin(const UserObject & y)
{
  const auto & other = static_cast<const NEML2FEInterpolation &>(y);
  mooseAssert(_fem_context_up_to_date == other._fem_context_up_to_date,
              "NEML2FEInterpolation becomes out of sync with other thread");

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
NEML2FEInterpolation::execute()
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
NEML2FEInterpolation::finalize()
{
  TIME_SECTION("finalize", 1, "Updating FEM context and interpolations for NEML2");

  if (!_fem_context_up_to_date)
    updateFEMContext();

  if (!_interp_up_to_date)
    updateInterpolations();
}

void
NEML2FEInterpolation::updateFEMContext()
{
  TIME_SECTION("updateFEMContext", 2, "Updating FEM context for NEML2");

  updateDofMap();
  updatePhi();
  updateGradPhi();

  // done
  _fem_context_up_to_date = true;
}

void
NEML2FEInterpolation::updateDofMap()
{
  auto device = _app.getLibtorchDevice();
  auto nelem = _neml2_assembly.numElem();

  for (auto & [var_name, moose_dof_map] : _moose_dof_map)
  {
    auto var = _moose_vars.at(var_name);
    auto ndofe = _ndofe.at(var->feType());

    // sanity check on sizes
    if (moose_dof_map.size() != std::size_t(nelem * ndofe))
      mooseError(
          "dof map size mismatch, expected ", nelem * ndofe, " but got ", moose_dof_map.size());

    // convert to neml2 tensor
    _neml2_dof_map[var_name] =
        neml2::Tensor(at::from_blob(moose_dof_map.data(), {nelem, ndofe}, torch::kInt64), 2)
            .to(device);

    _local_ndof = std::max(_local_ndof, _neml2_dof_map[var_name].max().item<int64_t>() + 1);
  }
}

void
NEML2FEInterpolation::updatePhi()
{
  auto device = _app.getLibtorchDevice();
  auto nelem = _neml2_assembly.numElem();
  auto nqp = _neml2_assembly.numQP();

  for (auto & [fetype, moose_phi] : _moose_phi)
  {
    auto ndofe = _ndofe.at(fetype);

    // sanity check on sizes
    if (moose_phi.size() != std::size_t(nelem * ndofe * nqp))
      mooseError("shape function size mismatch, expected ",
                 nelem * ndofe * nqp,
                 " but got ",
                 moose_phi.size());
    _neml2_phi[fetype] =
        neml2::Tensor(at::from_blob(moose_phi.data(), {nelem, ndofe, nqp}, torch::kFloat64), 3)
            .to(device);
  }
}

void
NEML2FEInterpolation::updateGradPhi()
{
  auto device = _app.getLibtorchDevice();
  auto nelem = _neml2_assembly.numElem();
  auto nqp = _neml2_assembly.numQP();

  for (auto & [fetype, moose_grad_phi] : _moose_grad_phi)
  {
    auto ndofe = _ndofe.at(fetype);

    // sanity check on sizes
    if (moose_grad_phi.size() != std::size_t(nelem * ndofe * nqp * 3))
      mooseError("shape function gradient size mismatch, expected ",
                 nelem * ndofe * nqp * 3,
                 " but got ",
                 moose_grad_phi.size());
    _neml2_grad_phi[fetype] =
        neml2::Tensor(at::from_blob(moose_grad_phi.data(), {nelem, ndofe, nqp, 3}, torch::kFloat64),
                      3)
            .to(device);
  }
}

void
NEML2FEInterpolation::updateInterpolations()
{
  TIME_SECTION("updateInterpolations", 2, "Updating FEM interpolations for NEML2");

  // convert the local solution vector to neml2 tensor
  auto sol = at::from_blob(const_cast<Real *>(_petsc_solution->get_array_read()),
                           {local_ndof()},
                           torch::kFloat64)
                 .to(_app.getLibtorchDevice());

  // interpolate variable values
  for (auto & [var_name, val] : _vars)
  {
    const auto & dof_map = _neml2_dof_map[var_name];
    const auto fetype = _moose_vars[var_name]->feType();
    const auto & phi = _neml2_phi[fetype];
    auto sol_scattered = neml2::discretization::scatter(sol, dof_map);
    val = neml2::discretization::interpolate(sol_scattered, phi);
  }

  // interpolate variable gradients
  for (auto & [var_name, val] : _grad_vars)
  {
    const auto & dof_map = _neml2_dof_map[var_name];
    const auto fetype = _moose_vars[var_name]->feType();
    const auto & grad_phi = _neml2_grad_phi[fetype];
    auto sol_scattered = neml2::discretization::scatter(sol, dof_map);
    val = neml2::discretization::interpolate(sol_scattered, grad_phi);
  }

  // close solution and residual vector access
  const_cast<PetscVector<Real> *>(_petsc_solution)->restore_array();

  // done
  _interp_up_to_date = true;
}

#endif
