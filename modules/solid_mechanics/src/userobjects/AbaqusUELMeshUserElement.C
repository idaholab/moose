//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AbaqusUELMeshUserElement.h"
#include "AbaqusUELMesh.h"
#include "MooseTypes.h"
#include "SystemBase.h"
#include "Executioner.h"

#define QUOTE(macro) stringifyName(macro)

registerMooseObject("SolidMechanicsApp", AbaqusUELMeshUserElement);

InputParameters
AbaqusUELMeshUserElement::validParams()
{
  auto params = GeneralUserObject::validParams();
  params += TaggingInterface::validParams();
  params.addClassDescription("Coupling UserObject to use Abaqus UEL plugins in MOOSE");

  // execute during residual and Jacobian evaluation
  ExecFlagEnum & exec_enum = params.set<ExecFlagEnum>("execute_on", true);
  exec_enum.addAvailableFlags(EXEC_PRE_KERNELS);
  exec_enum = {EXEC_PRE_KERNELS};
  params.suppressParameter<ExecFlagEnum>("execute_on");

  // Avoid uninitialized residual objects
  params.suppressParameter<bool>("force_preic");

  // UEL type
  params.addRequiredParam<std::string>("uel_type", "UEL type name (from the Abaqus .inp file)");

  // UEL element sets this object applies to
  params.addRequiredParam<std::vector<std::string>>("element_sets",
                                                    "Abaqus element sets this object applies to.");

  // UEL plugin file
  params.addRequiredParam<FileName>("plugin", "UEL plugin file");

  params.addParam<bool>(
      "use_energy", false, "Set to true of the UEL plugin makes use of the ENERGY parameter");

  params.addRelationshipManager("AbaqusUELRelationshipManager",
                                Moose::RelationshipManagerType::GEOMETRIC |
                                    Moose::RelationshipManagerType::ALGEBRAIC |
                                    Moose::RelationshipManagerType::COUPLING);
  return params;
}

AbaqusUELMeshUserElement::AbaqusUELMeshUserElement(const InputParameters & params)
  : GeneralUserObject(params),
    TaggingInterface(this),
    _uel_type(getParam<std::string>("uel_type")),
    _plugin(getParam<FileName>("plugin")),
    _library(_plugin + std::string("-") + QUOTE(METHOD) + ".plugin"),
    _uel(_library.getFunction<uel_t>("uel_")),
    _uel_mesh(
        [this]() -> AbaqusUELMesh &
        {
          auto * uel_mesh = dynamic_cast<AbaqusUELMesh *>(&UserObject::_subproblem.mesh());
          if (!uel_mesh)
            mooseError("AbaqusUELMeshUserElement requires an AbaqusUELMesh to be used.");
          return *uel_mesh;
        }()),
    _uel_definition(_uel_mesh.getUEL(_uel_type)),
    _uel_elements(_uel_mesh.getElements({})),
    _element_set_names(getParam<std::vector<std::string>>("element_sets")),
    _variables(_uel_definition._n_nodes),
    _nstatev(_uel_definition._n_statev),
    _statev(declareRestartableData<
            std::array<std::unordered_map<Abaqus::AbaqusID, std::vector<Real>>, 2>>("statev")),
    _statev_index_current(declareRestartableData<std::size_t>("statev_index_current")),
    _statev_index_old(declareRestartableData<std::size_t>("statev_index_old")),
    _use_energy(getParam<bool>("use_energy")),
    _energy(declareRestartableData<std::map<dof_id_type, std::array<Real, 8>>>("energy")),
    _energy_old(declareRestartableData<std::map<dof_id_type, std::array<Real, 8>>>("energy_old"))
{
  // get coupled variables from UEL type
  for (const auto i : make_range(_uel_definition._n_nodes))
    for (const auto var : _uel_definition._vars[i])
      _variables[i].push_back(
          &UserObject::_subproblem.getVariable(0,
                                               _uel_mesh.getVarName(var),
                                               Moose::VarKindType::VAR_SOLVER,
                                               Moose::VarFieldType::VAR_FIELD_STANDARD));

  // We set up the list of elements once for now. In the future, if the sets can change, we need to
  // call this at every timestep.
  setupElementSet();
}

void
AbaqusUELMeshUserElement::setupElementSet()
{
  _active_elements.clear();
  const auto & elsets = _uel_mesh.getElementSets();
  // use set to avoid duplicate elements
  std::set<std::size_t> selected_elements;
  for (const auto & elset_name : _element_set_names)
  {
    const auto elset = elsets.find(elset_name);
    if (elset == elsets.end())
      mooseError("Element set '", elset_name, "' not found in UEL mesh");
    for (const auto uel_elem_index : elset->second)
    {
      // TODO: check pid! && _uel_elements[uel_elem_id].pid == processor_id()
      if (_uel_elements[uel_elem_index]._uel._type_id == _uel_definition._type_id)
        selected_elements.insert(uel_elem_index);
    }
  }
  _active_elements.assign(selected_elements.begin(), selected_elements.end());
}

void
AbaqusUELMeshUserElement::timestepSetup()
{
  // swap the current and old state data after a converged timestep
  if (_app.getExecutioner()->lastSolveConverged())
  {
    std::swap(_statev_index_old, _statev_index_current);
    if (_use_energy)
      for (const auto & [key, value] : _energy)
        _energy_old[key] = value;
  }
  else
  {
    // last timestep did not converge, restore energy from energy_old
    if (_use_energy)
      for (const auto & [key, value] : _energy_old)
        _energy[key] = value;
  }
}

void
AbaqusUELMeshUserElement::execute()
{
  mooseInfoRepeated("AbaqusUELMeshUserElement::execute()");

  // dof indices of all coupled variables
  std::vector<dof_id_type> var_dof_indices;
  std::vector<dof_id_type> all_dof_indices;
  std::vector<Real> all_dof_values;
  std::vector<Real> all_dof_increments;
  std::vector<Real> all_udot_dof_values;
  std::vector<Real> all_udotdot_dof_values;

  // parameters for the UEL plugin
  std::array<int, 5> lflags;
  int dim = _uel_definition._coords;

  std::vector<Real> coords(_uel_definition._n_nodes * dim);

  DenseVector<Real> local_re;
  DenseMatrix<Real> local_ke;

  std::array<Real, 8> dummy_energy;

  Real dt = _fe_problem.dt();
  Real pnewdt = dt; // ?
  _pnewdt = pnewdt; // ?
  Real time = _fe_problem.time();
  std::vector<Real> times{time - dt, time - dt}; // first entry should be the step time (TODO)

  // loop over active element sets
  for (const auto & uel_elem_index : _active_elements)
  {

    auto & uel_elem = _uel_elements[uel_elem_index];
    mooseAssert(_uel_definition._n_nodes == uel_elem._nodes.size(),
                "Inconsistent node numbers between element and UEL definition");

    // clear dof indices (TODO: consider caching those for all UEL elements!)
    all_dof_indices.clear();

    // get the list of dofs, looping over nodes first
    for (const auto i : make_range(_uel_definition._n_nodes))
    {
      const auto * node_elem = _uel_mesh.elemPtr(uel_elem._nodes[i]);
      mooseAssert(node_elem, "Node element not found for UEL element");

      // loop over variables at the node
      for (const auto j : index_range(_uel_definition._vars[i]))
      {
        _variables[i][j]->getDofIndices(node_elem, var_dof_indices);
        mooseAssert(var_dof_indices.size() == 1,
                    "Each variable should have exactly one DOF at each node element");
        all_dof_indices.push_back(var_dof_indices[0]);
      }

      // copy coords
      const auto & p = node_elem->point(0);
      for (const auto j : make_range(dim))
        coords[j + dim * i] = p(j);
    }

    int ndofel = all_dof_indices.size();

    // Get solution values
    all_dof_values.resize(ndofel);
    all_dof_increments.resize(ndofel);

    _sys.currentSolution()->get(all_dof_indices, all_dof_increments);
    _sys.solutionOld().get(all_dof_indices, all_dof_values);

    mooseAssert(all_dof_values.size() == all_dof_increments.size(), "Inconsistent solution size.");
    for (const auto i : index_range(all_dof_values))
      all_dof_increments[i] -= all_dof_values[i];

    all_udot_dof_values.resize(ndofel);
    all_udotdot_dof_values.resize(ndofel);

    // Get u_dot and u_dotdot solution values (required for future expansion of the interface)
    if (false)
    {
      all_udot_dof_values.resize(ndofel);
      _sys.solutionUDot()->get(all_dof_indices, all_udot_dof_values);
      all_udotdot_dof_values.resize(ndofel);
      _sys.solutionUDot()->get(all_dof_indices, all_udotdot_dof_values);
    }

    const bool do_residual = _sys.hasVector(_sys.residualVectorTag());
    const bool do_jacobian = _sys.hasMatrix(_sys.systemMatrixTag());

    // set flags
    if (do_residual && do_jacobian)
      lflags[2] = 0;
    else if (!do_residual && do_jacobian)
      lflags[2] = 4;
    else if (do_residual && !do_jacobian)
      lflags[2] = 5;
    else
      lflags[2] = 0;

    // prepare residual and jacobian storage
    local_re.resize(ndofel);
    local_ke.resize(ndofel, ndofel);

    int jtype = _uel_definition._type_id;
    int jelem = uel_elem._id;
    int nnode = _uel_definition._n_nodes;
    int nrhs = 1; // : RHS should contain the residual vector

    // dummy vars
    Real rdummy = 0;
    int idummy = 0;

    // make sure stateful data storage is sized correctly
    _statev[_statev_index_current][jelem].resize(_nstatev);

    // call the plugin
    _uel(local_re.get_values().data(),
         local_ke.get_values().data(),
         _statev[_statev_index_current][jelem].data(),
         _use_energy ? _energy[jelem].data() : dummy_energy.data(),
         &ndofel,
         &nrhs,
         &_nstatev,
         uel_elem._properties.first, // fortran forces us to do terrible things
         const_cast<int *>(&_uel_definition._n_properties),
         coords.data(),
         &dim,
         &nnode,
         all_dof_values.data() /* U[] */,
         all_dof_increments.data() /* DU[] */,
         all_udot_dof_values.data() /* V[] */,
         all_udotdot_dof_values.data() /* A[] */,
         &jtype, ///???? what does this ID even mean?
         times.data(),
         &dt,
         &idummy /* KSTEP */,
         &idummy /* KINC */,
         &jelem,
         nullptr /* PARAMS[] */,
         &idummy /* NDLOAD */,
         nullptr /* JDLTYP[] */,
         nullptr /* ADLMAG[] */,
         nullptr, //_aux_var_values_to_uel.data() /* PREDEF[] */,
         &idummy /* NPREDF */,
         lflags.data() /* LFLAGS[] */,
         &ndofel /* MLVARX */,
         nullptr /* DDLMAG[] */,
         &idummy /* MDLOAD */,
         &pnewdt,
         uel_elem._properties.second,
         const_cast<int *>(&_uel_definition._n_iproperties),
         &rdummy /* PERIOD */
    );

    if (pnewdt < _pnewdt && pnewdt != 0.0)
      _pnewdt = pnewdt;

    // write to the residual vector
    // sign of 'residuals' has been tested with external loading and matches that of moose-umat
    // setups.
    if (do_residual)
      addResiduals(_fe_problem.assembly(_tid, _sys.number()), local_re, all_dof_indices, -1.0);

    // write to the Jacobian (hope we don't have to transpose...)
    if (do_jacobian)
      addJacobian(_fe_problem.assembly(_tid, _sys.number()),
                  local_ke,
                  all_dof_indices,
                  all_dof_indices,
                  -1.0);
  }

  _sys.solution().close();
}

// const std::array<Real, 8> *
// AbaqusUELMeshUserElement::getUELEnergy(dof_id_type element_id) const
// {
//   const auto it = _energy.find(element_id);

//   // if this UO does not have the data for the requested element we return null
//   // this allows the querying object to try multiple (block restricted) AbaqusUELMeshUserElement
//   // user objects until it finds the value (or else error out)
//   if (it == _energy.end())
//     return nullptr;

//   return &it->second;
// }
