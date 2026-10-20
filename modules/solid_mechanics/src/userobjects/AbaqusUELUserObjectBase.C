//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AbaqusUELUserObjectBase.h"
#include "AbaqusInputObjects.h"
#include "Executioner.h"

#include <limits>
#include <unordered_map>

#define QUOTE(macro) stringifyName(macro)

template <typename Key, template <typename...> class Map>
InputParameters
AbaqusUELUserObjectBase<Key, Map>::validParams()
{
  auto params = GeneralUserObject::validParams();
  params += TaggingInterface::validParams();

  // execute during residual and Jacobian evaluation
  ExecFlagEnum & exec_enum = params.set<ExecFlagEnum>("execute_on", true);
  exec_enum.addAvailableFlags(EXEC_PRE_KERNELS);
  exec_enum = {EXEC_PRE_KERNELS};
  params.suppressParameter<ExecFlagEnum>("execute_on");

  // Avoid uninitialized residual objects
  params.suppressParameter<bool>("force_preic");

  // auxiliary variables (including temperature)
  params.addParam<std::vector<AuxVariableName>>(
      "external_fields",
      {},
      "Auxiliary field variables (or 'predefined field variables') passed to the UEL plugin. Some "
      "plugins may assume that the first field is temperature when there are multiple external "
      "fields.");

  // UEL plugin file
  params.addRequiredParam<FileName>("plugin", "UEL plugin file");

  params.addParam<bool>(
      "use_energy", false, "Set to true if the UEL plugin makes use of the ENERGY parameter");

  return params;
}

template <typename Key, template <typename...> class Map>
AbaqusUELUserObjectBase<Key, Map>::AbaqusUELUserObjectBase(const InputParameters & params,
                                                           int nstatev)
  : GeneralUserObject(params),
    TaggingInterface(this),
    _plugin(getParam<FileName>("plugin")),
    _library(_plugin + std::string("-") + QUOTE(METHOD) + ".plugin"),
    _uel(_library.getFunction<uel_t>("uel_")),
    _nstatev(nstatev),
    _statev(declareRestartableData<std::array<Map<Key, std::vector<Real>>, 2>>("statev")),
    _statev_index_current(declareRestartableData<std::size_t>("statev_index_current", 0)),
    _statev_index_old(declareRestartableData<std::size_t>("statev_index_old", 1)),
    _t_step_old(declareRestartableData<int>("uel_tstep_old", -1)),
    _use_energy(getParam<bool>("use_energy")),
    _energy(declareRestartableData<std::map<Key, std::array<Real, 8>>>("energy")),
    _energy_old(declareRestartableData<std::map<Key, std::array<Real, 8>>>("energy_old")),
    _pnewdt(std::numeric_limits<Real>::max())
{
}

template <typename Key, template <typename...> class Map>
void
AbaqusUELUserObjectBase<Key, Map>::timestepSetup()
{
  // In case we are on the same timestep, that means this is being redone and we
  // don't want to swap the stateful data yet, but we restore the energy from energy_old
  if (_t_step == _t_step_old)
  {
    if (_use_energy)
      _energy = _energy_old;
    return;
  }

  // swap the current and old state data after a converged timestep
  if (_app.getExecutioner()->lastSolveConverged())
  {
    std::swap(_statev_index_old, _statev_index_current);
    if (_use_energy)
      _energy_old = _energy;
    _t_step_old = _t_step;
  }
}

template <typename Key, template <typename...> class Map>
const std::array<Real, 8> *
AbaqusUELUserObjectBase<Key, Map>::getUELEnergy(Key element_id) const
{
  const auto it = _energy.find(element_id);

  // if this UO does not have the data for the requested element we return null
  // this allows the querying object to try multiple (block restricted or element set restricted)
  // UEL user objects until it finds the value (or else error out)
  if (it == _energy.end())
    return nullptr;

  return &it->second;
}

template <typename Key, template <typename...> class Map>
int
AbaqusUELUserObjectBase<Key, Map>::computeLFlag3(bool do_residual, bool do_jacobian)
{
  if (do_residual && do_jacobian)
    return 0;
  else if (!do_residual && do_jacobian)
    return 4;
  else if (do_residual && !do_jacobian)
    return 5;
  else
    return 0;
}

template <typename Key, template <typename...> class Map>
void
AbaqusUELUserObjectBase<Key, Map>::updatePNewDt(Real & current_min, Real pnewdt)
{
  if (pnewdt < current_min && pnewdt != 0.0)
    current_min = pnewdt;
}

template <typename Key, template <typename...> class Map>
void
AbaqusUELUserObjectBase<Key, Map>::assembleUEL(Assembly & assembly,
                                               const DenseVector<Real> & re,
                                               const DenseMatrix<Real> & ke,
                                               DenseMatrix<Real> & ke_T,
                                               const std::vector<dof_id_type> & dofs,
                                               bool do_residual,
                                               bool do_jacobian)
{
  // write to the residual vector
  // sign of 'residuals' has been tested with external loading and matches that of moose-umat
  // setups.
  if (do_residual)
    addResiduals(assembly, re, dofs, -1.0);

  // write to the Jacobian (AMATRX is column-major, so transpose it first)
  if (do_jacobian)
  {
    ke.get_transpose(ke_T);
    addJacobian(assembly, ke_T, dofs, dofs, 1.0);
  }
}

template class AbaqusUELUserObjectBase<dof_id_type, std::map>;
template class AbaqusUELUserObjectBase<Abaqus::Index, std::unordered_map>;
