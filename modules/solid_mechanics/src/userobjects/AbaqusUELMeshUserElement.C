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
#include "AbaqusDLoadInterpolator.h"
#include "MooseError.h"
#include "MooseTypes.h"
#include "SystemBase.h"
#include "AuxiliarySystem.h"
#include <limits>

registerMooseObject("SolidMechanicsApp", AbaqusUELMeshUserElement);

InputParameters
AbaqusUELMeshUserElement::validParams()
{
  auto params = AbaqusUELUserObjectBase<Abaqus::Index, std::unordered_map>::validParams();
  params.addClassDescription(
      "Calls an Abaqus UEL plugin on the user elements of the selected "
      "AbaqusUELMesh element sets and assembles their residual and Jacobian.");
  params.set<MultiMooseEnum>("vector_tags").setAdditionalValue("AbaqusUELTag");

  // UEL type
  params.addRequiredParam<std::string>("uel_type", "UEL type name (from the Abaqus .inp file)");

  // UEL element sets this object applies to
  params.addRequiredParam<std::vector<std::string>>("element_sets",
                                                    "Abaqus element sets this object applies to.");

  // DLOAD interpolator (provides per-element arrays); default to 'dload_uo'
  params.addParam<UserObjectName>(
      "dload_interpolator", "dload_uo", "DLOAD interpolator user object");

  params.addRelationshipManager("AbaqusUELRelationshipManager",
                                Moose::RelationshipManagerType::GEOMETRIC |
                                    Moose::RelationshipManagerType::ALGEBRAIC |
                                    Moose::RelationshipManagerType::COUPLING);
  return params;
}

AbaqusUELMeshUserElement::AbaqusUELMeshUserElement(const InputParameters & params)
  : AbaqusUELUserObjectBase<Abaqus::Index, std::unordered_map>(
        params, getUELMesh(params).getUEL(params.get<std::string>("uel_type"))._n_statev),
    _aux_sys(_fe_problem.getAuxiliarySystem()),
    _uel_type(getParam<std::string>("uel_type")),
    _uel_mesh(getUELMesh(params)),
    _uel_definition(_uel_mesh.getUEL(_uel_type)),
    _uel_elements(_uel_mesh.getElements()),
    _element_set_names(getParam<std::vector<std::string>>("element_sets")),
    _aux_variable_names(getParam<std::vector<AuxVariableName>>("external_fields")),
    _variables(_uel_definition._n_nodes),
    _dload_uo(getUserObject<AbaqusDLoadInterpolator>("dload_interpolator"))
{
  // get coupled variables from UEL type
  for (const auto i : make_range(_uel_definition._n_nodes))
    for (const auto var : _uel_definition._vars[i])
      _variables[i].push_back(
          &UserObject::_subproblem.getVariable(0,
                                               _uel_mesh.getVarName(var),
                                               Moose::VarKindType::VAR_SOLVER,
                                               Moose::VarFieldType::VAR_FIELD_STANDARD));

  // get PREDEF fields in the form of AuxVariables (set those using AbaqusPredefAux for example)
  for (const auto & aux_variable_name : _aux_variable_names)
  {
    MooseVariableFEBase * aux_var =
        &UserObject::_subproblem.getVariable(0,
                                             aux_variable_name,
                                             Moose::VarKindType::VAR_AUXILIARY,
                                             Moose::VarFieldType::VAR_FIELD_STANDARD);
    _aux_variables.push_back(aux_var);

    // TODO: check block restriction
  }

  // We set up the list of elements once for now. In the future, if the sets can change, we need to
  // call this at every timestep.
  setupElementSet();
}

AbaqusUELMesh &
AbaqusUELMeshUserElement::getUELMesh(const InputParameters & params)
{
  auto * uel_mesh = dynamic_cast<AbaqusUELMesh *>(
      &params.getCheckedPointerParam<SubProblem *>("_subproblem")->mesh());
  if (!uel_mesh)
    params.mooseError("AbaqusUELMeshUserElement requires an AbaqusUELMesh to be used.");
  return *uel_mesh;
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
      const auto & uel_elem = _uel_elements[uel_elem_index];
      if (uel_elem._uel._type_id != _uel_definition._type_id)
        continue;

      // Process each UEL element on exactly one rank: the one that owns its first node-element.
      // The AbaqusUELRelationshipManager (geometric/algebraic/coupling) ghosts the remaining
      // node-elements - and their dofs - of every UEL element that touches a locally owned
      // node-element, so the owning rank can always read the full element solution. Without this
      // filter every rank would process every element and read non-ghosted off-processor dofs.
      // On a distributed mesh a node-element that is neither local nor ghosted is absent here, and
      // then this rank does not own it.
      const auto * first_node_elem = _uel_mesh.queryElemPtr(uel_elem._nodes[0]);
      if (first_node_elem && first_node_elem->processor_id() == processor_id())
        selected_elements.insert(uel_elem_index);
    }
  }
  _active_elements.assign(selected_elements.begin(), selected_elements.end());
}

void
AbaqusUELMeshUserElement::execute()
{
  // dof indices of all coupled variables
  std::vector<dof_id_type> var_dof_indices;
  std::vector<dof_id_type> all_dof_indices;
  std::vector<Real> all_dof_values;
  std::vector<Real> all_dof_increments;
  std::vector<Real> all_udot_dof_values;
  std::vector<Real> all_udotdot_dof_values;

  // vectors for aux variables
  std::vector<dof_id_type> aux_var_dof_indices;
  std::vector<dof_id_type> all_aux_var_dof_indices;
  std::vector<Real> all_aux_var_dof_values;
  std::vector<Real> aux_var_values_to_uel;

  // dload arrays assembled by interpolator at time step start
  std::vector<int> jdltp_vec;
  std::vector<Real> adlmag_vec;

  // parameters for the UEL plugin
  std::array<int, 5> lflags{};
  // static analysis
  lflags[0] = 1;
  // small displacement analysis
  lflags[1] = 0;
  // general solve step
  lflags[3] = 0;
  // newton based solution
  lflags[4] = 0;

  int dim = _uel_definition._coords;
  int nprops = _uel_definition._n_properties;
  int njprop = _uel_definition._n_iproperties;

  std::vector<Real> coords(_uel_definition._n_nodes * dim);

  std::array<Real, 8> energy;

  Real dt = _fe_problem.dt();
  _pnewdt = std::numeric_limits<Real>::max();
  Real time = _fe_problem.time();
  std::vector<Real> times{time - dt, time - dt}; // first entry should be the step time (TODO)

  // loop over active element sets
  for (const auto & uel_elem_index : _active_elements)
  {

    const auto & uel_elem = _uel_elements[uel_elem_index];
    mooseAssert(_uel_definition._n_nodes == uel_elem._nodes.size(),
                "Inconsistent node numbers between element and UEL definition");

    // clear dof indices (TODO: consider caching those for all UEL elements!)
    all_dof_indices.clear();

    int nnode = _uel_definition._n_nodes;

    // Prepare external fields
    const auto nvar_aux = _aux_variables.size();
    all_aux_var_dof_indices.resize(nnode * nvar_aux);

    // get the list of dofs, looping over nodes first
    for (const auto i : make_range(nnode))
    {
      const auto * node_elem = _uel_mesh.elemPtr(uel_elem._nodes[i]);
      mooseAssert(node_elem, "Node element not found for UEL element");

      // get DOFs for all variables at the node
      for (const auto j : index_range(_uel_definition._vars[i]))
      {
        _variables[i][j]->getDofIndices(node_elem, var_dof_indices);
        mooseAssert(var_dof_indices.size() == 1,
                    "Each variable should have exactly one DOF at each node element");
        all_dof_indices.push_back(var_dof_indices[0]);
      }

      // get DOFs for predefined fields (AuxVariables)
      for (const auto j : index_range(_aux_variables))
      {
        _aux_variables[j]->getDofIndices(node_elem, aux_var_dof_indices);

        mooseAssert(aux_var_dof_indices.size() == 1,
                    "Each aux variable should have exactly one DOF at each node element");

        all_aux_var_dof_indices[i * nvar_aux + j] = aux_var_dof_indices[0];
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

    _sys.currentSolution()->get(all_dof_indices, all_dof_values);
    _sys.solutionOld().get(all_dof_indices, all_dof_increments);

    mooseAssert(all_dof_values.size() == all_dof_increments.size(), "Inconsistent solution size.");
    for (const auto i : index_range(all_dof_values))
      all_dof_increments[i] = all_dof_values[i] - all_dof_increments[i];

    all_udot_dof_values.resize(ndofel);
    all_udotdot_dof_values.resize(ndofel);

    // Prepare external field values
    if (nvar_aux)
    {
      all_aux_var_dof_values.resize(nnode * nvar_aux);
      aux_var_values_to_uel.resize(nnode * nvar_aux * 2); // Value _and_ increment

      _aux_sys.currentSolution()->get(all_aux_var_dof_indices, all_aux_var_dof_values);

      // First, one external field and increment; then, second external field and increment, etc.
      // Only the current field value is supported, so every field increment is passed as zero.
      for (const auto i : index_range(all_aux_var_dof_values))
      {
        aux_var_values_to_uel[i * 2] = all_aux_var_dof_values[i];
        aux_var_values_to_uel[i * 2 + 1] = 0.0;
      }
    }

    const bool do_residual = _sys.hasVector(_sys.residualVectorTag());
    const bool do_jacobian = _sys.hasMatrix(_sys.systemMatrixTag());

    // set flags
    lflags[2] = computeLFlag3(do_residual, do_jacobian);

    // prepare residual and jacobian storage
    _local_re.resize(ndofel);
    _local_ke.resize(ndofel, ndofel);

    // make sure stateful data storage is sized correctly
    auto & current_state = _statev[_statev_index_current][uel_elem_index];
    current_state = _statev[_statev_index_old][uel_elem_index];
    current_state.resize(_nstatev);

    // the plugin receives the energy at the start of the increment
    energy = _use_energy ? _energy_old[uel_elem_index] : std::array<Real, 8>{};

    // Assemble distributed loads for this element from DLoadInterpolator
    int ndload_val = 0;
    int mdload_val = 0;
    if (const auto * tptr = _dload_uo.getTypes(uel_elem_index))
    {
      const auto * mptr = _dload_uo.getMagnitudes(uel_elem_index);
      if (mptr && tptr->size() == mptr->size() && !tptr->empty())
      {
        jdltp_vec = *tptr;
        adlmag_vec = *mptr;
        ndload_val = static_cast<int>(jdltp_vec.size());
        mdload_val = ndload_val;
      }
    }

    // call the plugin
    Abaqus::UELArguments args;
    args.rhs = _local_re.get_values().data();
    args.amatrx = _local_ke.get_values().data();
    args.svars = current_state.data();
    args.energy = energy.data();
    args.ndofel = ndofel;
    args.nrhs = 1; // RHS should contain the residual vector
    args.nsvars = _nstatev;
    args.props = uel_elem._properties.first; // fortran forces us to do terrible things :)
    args.nprops = nprops;
    args.coords = coords.data();
    args.mcrd = dim;
    args.nnode = nnode;
    args.u = all_dof_values.data();
    args.du = all_dof_increments.data();
    args.v = all_udot_dof_values.data();
    args.a = all_udotdot_dof_values.data();
    args.jtype = _uel_definition._type_id;
    args.time = times.data();
    args.dtime = dt;
    args.kstep = 0;
    args.kinc = 0;
    args.jelem = uel_elem._id;
    args.params = nullptr;
    args.ndload = ndload_val;
    args.jdltyp = ndload_val ? jdltp_vec.data() : nullptr;
    args.adlmag = ndload_val ? adlmag_vec.data() : nullptr;
    args.predef = aux_var_values_to_uel.data();
    args.npredf = nvar_aux; // Number of external fields.
    args.lflags = lflags.data();
    args.mlvarx = ndofel;
    args.ddlmag = nullptr;
    args.mdload = mdload_val;
    // Abaqus initializes PNEWDT to a large value before each call
    args.pnewdt = std::numeric_limits<Real>::max();
    args.jprops = uel_elem._properties.second;
    args.njprop = njprop;
    args.period = 0;
    args.call(_uel);

    updatePNewDt(_pnewdt, args.pnewdt);

    if (_use_energy)
      _energy[uel_elem_index] = energy;

    assembleUEL(_fe_problem.assembly(_tid, _sys.number()),
                _local_re,
                _local_ke,
                _local_ke_T,
                all_dof_indices,
                do_residual,
                do_jacobian);
  }
}
