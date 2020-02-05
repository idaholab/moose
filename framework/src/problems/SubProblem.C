//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SubProblem.h"
#include "Factory.h"
#include "MooseMesh.h"
#include "Conversion.h"
#include "Function.h"
#include "MooseApp.h"
#include "MooseVariableFE.h"
#include "MooseArray.h"
#include "SystemBase.h"
#include "Assembly.h"

#include "libmesh/equation_systems.h"
#include "libmesh/system.h"
#include "libmesh/dof_map.h"

defineLegacyParams(SubProblem);

InputParameters
SubProblem::validParams()
{
  InputParameters params = Problem::validParams();

  params.addParam<bool>(
      "default_ghosting",
      false,
      "Whether or not to use libMesh's default amount of algebraic and geometric ghosting");

  params.addParamNamesToGroup("default_ghosting", "Advanced");

  return params;
}

// SubProblem /////
SubProblem::SubProblem(const InputParameters & parameters)
  : Problem(parameters),
    _factory(_app.getFactory()),
    _nonlocal_cm(),
    _requires_nonlocal_coupling(false),
    _default_ghosting(getParam<bool>("default_ghosting")),
    _rz_coord_axis(1), // default to RZ rotation around y-axis
    _currently_computing_jacobian(false),
    _computing_nonlinear_residual(false),
    _safe_access_tagged_matrices(false),
    _safe_access_tagged_vectors(false),
    _have_ad_objects(false)
{
  unsigned int n_threads = libMesh::n_threads();
  _active_elemental_moose_variables.resize(n_threads);
  _has_active_elemental_moose_variables.resize(n_threads);
  _active_material_property_ids.resize(n_threads);

  _active_fe_var_coupleable_matrix_tags.resize(n_threads);
  _active_fe_var_coupleable_vector_tags.resize(n_threads);
  _active_sc_var_coupleable_matrix_tags.resize(n_threads);
  _active_sc_var_coupleable_vector_tags.resize(n_threads);
}

SubProblem::~SubProblem() {}

TagID
SubProblem::addVectorTag(TagName tag_name)
{
  auto tag_name_upper = MooseUtils::toUpper(tag_name);
  auto existing_tag = _vector_tag_name_to_tag_id.find(tag_name_upper);
  if (existing_tag == _vector_tag_name_to_tag_id.end())
  {
    auto tag_id = _vector_tag_name_to_tag_id.size();

    _vector_tag_name_to_tag_id[tag_name_upper] = tag_id;

    _vector_tag_id_to_tag_name[tag_id] = tag_name_upper;
  }

  return _vector_tag_name_to_tag_id.at(tag_name_upper);
}

bool
SubProblem::vectorTagExists(const TagName & tag_name)
{
  auto tag_name_upper = MooseUtils::toUpper(tag_name);

  return _vector_tag_name_to_tag_id.find(tag_name_upper) != _vector_tag_name_to_tag_id.end();
}

TagID
SubProblem::getVectorTagID(const TagName & tag_name)
{
  auto tag_name_upper = MooseUtils::toUpper(tag_name);

  if (!vectorTagExists(tag_name))
    mooseError("Vector tag: ",
               tag_name,
               " does not exist. ",
               "If this is a TimeKernel then this may have happened because you didn't "
               "specify a Transient Executioner.");

  return _vector_tag_name_to_tag_id.at(tag_name_upper);
}

TagName
SubProblem::vectorTagName(TagID tag)
{
  return _vector_tag_id_to_tag_name[tag];
}

TagID
SubProblem::addMatrixTag(TagName tag_name)
{
  auto tag_name_upper = MooseUtils::toUpper(tag_name);
  auto existing_tag = _matrix_tag_name_to_tag_id.find(tag_name_upper);
  if (existing_tag == _matrix_tag_name_to_tag_id.end())
  {
    auto tag_id = _matrix_tag_name_to_tag_id.size();

    _matrix_tag_name_to_tag_id[tag_name_upper] = tag_id;

    _matrix_tag_id_to_tag_name[tag_id] = tag_name_upper;
  }

  return _matrix_tag_name_to_tag_id.at(tag_name_upper);
}

bool
SubProblem::matrixTagExists(const TagName & tag_name)
{
  auto tag_name_upper = MooseUtils::toUpper(tag_name);

  return _matrix_tag_name_to_tag_id.find(tag_name_upper) != _matrix_tag_name_to_tag_id.end();
}

bool
SubProblem::matrixTagExists(TagID tag_id)
{
  return _matrix_tag_id_to_tag_name.find(tag_id) != _matrix_tag_id_to_tag_name.end();
}

TagID
SubProblem::getMatrixTagID(const TagName & tag_name)
{
  auto tag_name_upper = MooseUtils::toUpper(tag_name);

  if (!matrixTagExists(tag_name))
    mooseError("Matrix tag: ",
               tag_name,
               " does not exist. ",
               "If this is a TimeKernel then this may have happened because you didn't "
               "specify a Transient Executioner.");

  return _matrix_tag_name_to_tag_id.at(tag_name_upper);
}

TagName
SubProblem::matrixTagName(TagID tag)
{
  return _matrix_tag_id_to_tag_name[tag];
}

void
SubProblem::setActiveFEVariableCoupleableMatrixTags(std::set<TagID> & mtags, THREAD_ID tid)
{
  _active_fe_var_coupleable_matrix_tags[tid] = mtags;
}

void
SubProblem::setActiveFEVariableCoupleableVectorTags(std::set<TagID> & vtags, THREAD_ID tid)
{
  _active_fe_var_coupleable_vector_tags[tid] = vtags;
}

void
SubProblem::clearActiveFEVariableCoupleableVectorTags(THREAD_ID tid)
{
  _active_fe_var_coupleable_vector_tags[tid].clear();
}

void
SubProblem::clearActiveFEVariableCoupleableMatrixTags(THREAD_ID tid)
{
  _active_fe_var_coupleable_matrix_tags[tid].clear();
}

const std::set<TagID> &
SubProblem::getActiveFEVariableCoupleableMatrixTags(THREAD_ID tid) const
{
  return _active_fe_var_coupleable_matrix_tags[tid];
}

const std::set<TagID> &
SubProblem::getActiveFEVariableCoupleableVectorTags(THREAD_ID tid) const
{
  return _active_fe_var_coupleable_vector_tags[tid];
}

void
SubProblem::setActiveScalarVariableCoupleableMatrixTags(std::set<TagID> & mtags, THREAD_ID tid)
{
  _active_sc_var_coupleable_matrix_tags[tid] = mtags;
}

void
SubProblem::setActiveScalarVariableCoupleableVectorTags(std::set<TagID> & vtags, THREAD_ID tid)
{
  _active_sc_var_coupleable_vector_tags[tid] = vtags;
}

void
SubProblem::clearActiveScalarVariableCoupleableVectorTags(THREAD_ID tid)
{
  _active_sc_var_coupleable_vector_tags[tid].clear();
}

void
SubProblem::clearActiveScalarVariableCoupleableMatrixTags(THREAD_ID tid)
{
  _active_sc_var_coupleable_matrix_tags[tid].clear();
}

const std::set<TagID> &
SubProblem::getActiveScalarVariableCoupleableMatrixTags(THREAD_ID tid) const
{
  return _active_sc_var_coupleable_matrix_tags[tid];
}

const std::set<TagID> &
SubProblem::getActiveScalarVariableCoupleableVectorTags(THREAD_ID tid) const
{
  return _active_sc_var_coupleable_vector_tags[tid];
}

void
SubProblem::setActiveElementalMooseVariables(const std::set<MooseVariableFEBase *> & moose_vars,
                                             THREAD_ID tid)
{
  if (!moose_vars.empty())
  {
    _has_active_elemental_moose_variables[tid] = 1;
    _active_elemental_moose_variables[tid] = moose_vars;
  }
}

const std::set<MooseVariableFEBase *> &
SubProblem::getActiveElementalMooseVariables(THREAD_ID tid) const
{
  return _active_elemental_moose_variables[tid];
}

bool
SubProblem::hasActiveElementalMooseVariables(THREAD_ID tid) const
{
  return _has_active_elemental_moose_variables[tid];
}

void
SubProblem::clearActiveElementalMooseVariables(THREAD_ID tid)
{
  _has_active_elemental_moose_variables[tid] = 0;
  _active_elemental_moose_variables[tid].clear();
}

void
SubProblem::setActiveMaterialProperties(const std::set<unsigned int> & mat_prop_ids, THREAD_ID tid)
{
  if (!mat_prop_ids.empty())
    _active_material_property_ids[tid] = mat_prop_ids;
}

const std::set<unsigned int> &
SubProblem::getActiveMaterialProperties(THREAD_ID tid) const
{
  return _active_material_property_ids[tid];
}

bool
SubProblem::hasActiveMaterialProperties(THREAD_ID tid) const
{
  return !_active_material_property_ids[tid].empty();
}

void
SubProblem::clearActiveMaterialProperties(THREAD_ID tid)
{
  _active_material_property_ids[tid].clear();
}

std::set<SubdomainID>
SubProblem::getMaterialPropertyBlocks(const std::string & prop_name)
{
  std::set<SubdomainID> blocks;

  for (const auto & it : _map_block_material_props)
  {
    const std::set<std::string> & prop_names = it.second;
    std::set<std::string>::iterator name_it = prop_names.find(prop_name);
    if (name_it != prop_names.end())
      blocks.insert(it.first);
  }

  return blocks;
}

std::vector<SubdomainName>
SubProblem::getMaterialPropertyBlockNames(const std::string & prop_name)
{
  std::set<SubdomainID> blocks = getMaterialPropertyBlocks(prop_name);
  std::vector<SubdomainName> block_names;
  block_names.reserve(blocks.size());
  for (const auto & block_id : blocks)
  {
    SubdomainName name;
    if (block_id == Moose::ANY_BLOCK_ID)
      name = "ANY_BLOCK_ID";
    else
    {
      name = mesh().getMesh().subdomain_name(block_id);
      if (name.empty())
      {
        std::ostringstream oss;
        oss << block_id;
        name = oss.str();
      }
    }
    block_names.push_back(name);
  }

  return block_names;
}

bool
SubProblem::hasBlockMaterialProperty(SubdomainID bid, const std::string & prop_name)
{
  auto it = _map_block_material_props.find(bid);
  if (it == _map_block_material_props.end())
    return false;

  if (it->second.count(prop_name) > 0)
    return true;
  else
    return false;
}

// TODO: remove code duplication by templating
std::set<BoundaryID>
SubProblem::getMaterialPropertyBoundaryIDs(const std::string & prop_name)
{
  std::set<BoundaryID> boundaries;

  for (const auto & it : _map_boundary_material_props)
  {
    const std::set<std::string> & prop_names = it.second;
    std::set<std::string>::iterator name_it = prop_names.find(prop_name);
    if (name_it != prop_names.end())
      boundaries.insert(it.first);
  }

  return boundaries;
}

std::vector<BoundaryName>
SubProblem::getMaterialPropertyBoundaryNames(const std::string & prop_name)
{
  std::set<BoundaryID> boundaries = getMaterialPropertyBoundaryIDs(prop_name);
  std::vector<BoundaryName> boundary_names;
  boundary_names.reserve(boundaries.size());
  const BoundaryInfo & boundary_info = mesh().getMesh().get_boundary_info();

  for (const auto & bnd_id : boundaries)
  {
    BoundaryName name;
    if (bnd_id == Moose::ANY_BOUNDARY_ID)
      name = "ANY_BOUNDARY_ID";
    else
    {
      name = boundary_info.get_sideset_name(bnd_id);
      if (name.empty())
      {
        std::ostringstream oss;
        oss << bnd_id;
        name = oss.str();
      }
    }
    boundary_names.push_back(name);
  }

  return boundary_names;
}

bool
SubProblem::hasBoundaryMaterialProperty(BoundaryID bid, const std::string & prop_name)
{
  auto it = _map_boundary_material_props.find(bid);
  if (it == _map_boundary_material_props.end())
    return false;

  if (it->second.count(prop_name) > 0)
    return true;
  else
    return false;
}

void
SubProblem::storeSubdomainMatPropName(SubdomainID block_id, const std::string & name)
{
  _map_block_material_props[block_id].insert(name);
}

void
SubProblem::storeBoundaryMatPropName(BoundaryID boundary_id, const std::string & name)
{
  _map_boundary_material_props[boundary_id].insert(name);
}

void
SubProblem::storeSubdomainZeroMatProp(SubdomainID block_id, const MaterialPropertyName & name)
{
  _zero_block_material_props[block_id].insert(name);
}

void
SubProblem::storeBoundaryZeroMatProp(BoundaryID boundary_id, const MaterialPropertyName & name)
{
  _zero_boundary_material_props[boundary_id].insert(name);
}

void
SubProblem::storeSubdomainDelayedCheckMatProp(const std::string & requestor,
                                              SubdomainID block_id,
                                              const std::string & name)
{
  _map_block_material_props_check[block_id].insert(std::make_pair(requestor, name));
}

void
SubProblem::storeBoundaryDelayedCheckMatProp(const std::string & requestor,
                                             BoundaryID boundary_id,
                                             const std::string & name)
{
  _map_boundary_material_props_check[boundary_id].insert(std::make_pair(requestor, name));
}

void
SubProblem::checkBlockMatProps()
{
  // Variable for storing the value for ANY_BLOCK_ID/ANY_BOUNDARY_ID
  SubdomainID any_id = Moose::ANY_BLOCK_ID;

  // Variable for storing all available blocks/boundaries from the mesh
  std::set<SubdomainID> all_ids(mesh().meshSubdomains());

  // Loop through the properties to check
  for (const auto & check_it : _map_block_material_props_check)
  {
    // The current id for the property being checked (BoundaryID || BlockID)
    SubdomainID check_id = check_it.first;

    // In the case when the material being checked has an ID is set to ANY, then loop through all
    // the possible ids and verify that the material property is defined.
    std::set<SubdomainID> check_ids = {check_id};
    if (check_id == any_id)
      check_ids = all_ids;

    // Loop through all the block/boundary ids
    for (const auto & id : check_ids)
    {
      // Loop through all the stored properties
      for (const auto & prop_it : check_it.second)
      {
        // Produce an error if the material property is not defined on the current block/boundary
        // and any block/boundary
        // and not is not a zero material property.
        if (_map_block_material_props[id].count(prop_it.second) == 0 &&
            _map_block_material_props[any_id].count(prop_it.second) == 0 &&
            _zero_block_material_props[id].count(prop_it.second) == 0 &&
            _zero_block_material_props[any_id].count(prop_it.second) == 0)
        {
          std::string check_name = restrictionSubdomainCheckName(id);
          if (check_name.empty())
            check_name = std::to_string(id);
          mooseError("Material property '",
                     prop_it.second,
                     "', requested by '",
                     prop_it.first,
                     "' is not defined on block ",
                     check_name);
        }
      }
    }
  }
}

void
SubProblem::checkBoundaryMatProps()
{
  // Variable for storing the value for ANY_BLOCK_ID/ANY_BOUNDARY_ID
  BoundaryID any_id = Moose::ANY_BOUNDARY_ID;

  // Variable for storing all available blocks/boundaries from the mesh
  std::set<BoundaryID> all_ids(mesh().getBoundaryIDs());

  // Loop through the properties to check
  for (const auto & check_it : _map_boundary_material_props_check)
  {
    // The current id for the property being checked (BoundaryID || BlockID)
    BoundaryID check_id = check_it.first;

    // In the case when the material being checked has an ID is set to ANY, then loop through all
    // the possible ids and verify that the material property is defined.
    std::set<BoundaryID> check_ids{check_id};
    if (check_id == any_id)
      check_ids = all_ids;

    // Loop through all the block/boundary ids
    for (const auto & id : check_ids)
    {
      // Loop through all the stored properties
      for (const auto & prop_it : check_it.second)
      {
        // Produce an error if the material property is not defined on the current block/boundary
        // and any block/boundary
        // and not is not a zero material property.
        if (_map_boundary_material_props[id].count(prop_it.second) == 0 &&
            _map_boundary_material_props[any_id].count(prop_it.second) == 0 &&
            _zero_boundary_material_props[id].count(prop_it.second) == 0 &&
            _zero_boundary_material_props[any_id].count(prop_it.second) == 0)
        {
          std::string check_name = restrictionBoundaryCheckName(id);
          if (check_name.empty())
            check_name = std::to_string(id);
          mooseError("Material property '",
                     prop_it.second,
                     "', requested by '",
                     prop_it.first,
                     "' is not defined on boundary ",
                     check_name);
        }
      }
    }
  }
}

void
SubProblem::markMatPropRequested(const std::string & prop_name)
{
  _material_property_requested.insert(prop_name);
}

bool
SubProblem::isMatPropRequested(const std::string & prop_name) const
{
  return _material_property_requested.find(prop_name) != _material_property_requested.end();
}

DiracKernelInfo &
SubProblem::diracKernelInfo()
{
  return _dirac_kernel_info;
}

Real
SubProblem::finalNonlinearResidual() const
{
  return 0;
}

unsigned int
SubProblem::nNonlinearIterations() const
{
  return 0;
}

unsigned int
SubProblem::nLinearIterations() const
{
  return 0;
}

void
SubProblem::meshChanged()
{
  mooseError("This system does not support changing the mesh");
}

std::string
SubProblem::restrictionSubdomainCheckName(SubdomainID check_id)
{
  // TODO: Put a better a interface in MOOSE
  std::map<subdomain_id_type, std::string> & name_map = mesh().getMesh().set_subdomain_name_map();
  std::map<subdomain_id_type, std::string>::const_iterator pos = name_map.find(check_id);
  if (pos != name_map.end())
    return pos->second;
  return "";
}

std::string
SubProblem::restrictionBoundaryCheckName(BoundaryID check_id)
{
  return mesh().getMesh().get_boundary_info().sideset_name(check_id);
}

void
SubProblem::setCurrentBoundaryID(BoundaryID bid, THREAD_ID tid)
{
  assembly(tid).setCurrentBoundaryID(bid);
}

unsigned int
SubProblem::getAxisymmetricRadialCoord() const
{
  if (_rz_coord_axis == 0)
    return 1; // if the rotation axis is x (0), then the radial direction is y (1)
  else
    return 0; // otherwise the radial direction is assumed to be x, i.e., the rotation axis is y
}

MooseVariableFEBase &
SubProblem::getVariableHelper(THREAD_ID tid,
                              const std::string & var_name,
                              Moose::VarKindType expected_var_type,
                              Moose::VarFieldType expected_var_field_type,
                              SystemBase & nl,
                              SystemBase & aux)
{
  // Eventual return value
  MooseVariableFEBase * var = nullptr;

  // First check that the variable is found on the expected system.
  if (expected_var_type == Moose::VarKindType::VAR_ANY)
  {
    if (nl.hasVariable(var_name))
      var = &(nl.getVariable(tid, var_name));
    else if (aux.hasVariable(var_name))
      var = &(aux.getVariable(tid, var_name));
    else
      mooseError("Unknown variable " + var_name);
  }
  else if (expected_var_type == Moose::VarKindType::VAR_NONLINEAR && nl.hasVariable(var_name))
    var = &(nl.getVariable(tid, var_name));
  else if (expected_var_type == Moose::VarKindType::VAR_AUXILIARY && aux.hasVariable(var_name))
    var = &(aux.getVariable(tid, var_name));
  else
  {
    std::string expected_var_type_string =
        (expected_var_type == Moose::VarKindType::VAR_NONLINEAR ? "nonlinear" : "auxiliary");
    mooseError("No ",
               expected_var_type_string,
               " variable named ",
               var_name,
               " found. "
               "Did you specify an auxiliary variable when you meant to specify a nonlinear "
               "variable (or vice-versa)?");
  }

  // Now make sure the var found has the expected field type.
  if ((expected_var_field_type == Moose::VarFieldType::VAR_FIELD_ANY) ||
      (expected_var_field_type == var->fieldType()))
    return *var;
  else
  {
    std::string expected_var_field_type_string =
        MooseUtils::toLower(Moose::stringify(expected_var_field_type));
    std::string var_field_type_string = MooseUtils::toLower(Moose::stringify(var->fieldType()));

    mooseError("No ",
               expected_var_field_type_string,
               " variable named ",
               var_name,
               " found. "
               "Did you specify a ",
               var_field_type_string,
               " variable when you meant to specify a ",
               expected_var_field_type_string,
               " variable?");
  }
}

void
SubProblem::reinitElemFaceRef(const Elem * elem,
                              unsigned int side,
                              BoundaryID bnd_id,
                              Real tolerance,
                              const std::vector<Point> * const pts,
                              const std::vector<Real> * const weights,
                              THREAD_ID tid)
{
  // - Set our _current_elem for proper dof index getting in the moose variables
  // - Reinitialize all of our FE objects so we have current phi, dphi, etc. data
  // Note that our number of shape functions will reflect the number of shapes associated with the
  // interior element while the number of quadrature points will be determined by the passed pts
  // parameter (which presumably will have a number of pts reflective of a facial quadrature rule)
  assembly(tid).reinitElemFaceRef(elem, side, tolerance, pts, weights);

  // Actually get the dof indices in the moose variables
  systemBaseNonlinear().prepare(tid);
  systemBaseAuxiliary().prepare(tid);

  // With the dof indices set in the moose variables, now let's properly size
  // our local residuals/Jacobians
  assembly(tid).prepareJacobianBlock();
  assembly(tid).prepareResidual();

  // Let's finally compute our variable values!
  systemBaseNonlinear().reinitElemFace(elem, side, bnd_id, tid);
  systemBaseAuxiliary().reinitElemFace(elem, side, bnd_id, tid);
}

void
SubProblem::reinitNeighborFaceRef(const Elem * neighbor_elem,
                                  unsigned int neighbor_side,
                                  BoundaryID bnd_id,
                                  Real tolerance,
                                  const std::vector<Point> * const pts,
                                  const std::vector<Real> * const weights,
                                  THREAD_ID tid)
{
  // - Set our _current_neighbor_elem for proper dof index getting in the moose variables
  // - Reinitialize all of our FE objects so we have current phi, dphi, etc. data
  // Note that our number of shape functions will reflect the number of shapes associated with the
  // interior element while the number of quadrature points will be determined by the passed pts
  // parameter (which presumably will have a number of pts reflective of a facial quadrature rule)
  assembly(tid).reinitNeighborFaceRef(neighbor_elem, neighbor_side, tolerance, pts, weights);

  // Actually get the dof indices in the moose variables
  systemBaseNonlinear().prepareNeighbor(tid);
  systemBaseAuxiliary().prepareNeighbor(tid);

  // With the dof indices set in the moose variables, now let's properly size
  // our local residuals/Jacobians
  assembly(tid).prepareNeighbor();

  // Let's finally compute our variable values!
  systemBaseNonlinear().reinitNeighborFace(neighbor_elem, neighbor_side, bnd_id, tid);
  systemBaseAuxiliary().reinitNeighborFace(neighbor_elem, neighbor_side, bnd_id, tid);
}

void
SubProblem::reinitLowerDElemRef(const Elem * elem,
                                const std::vector<Point> * const pts,
                                const std::vector<Real> * const weights,
                                THREAD_ID tid)
{
  // - Set our _current_lower_d_elem for proper dof index getting in the moose variables
  // - Reinitialize all of our lower-d FE objects so we have current phi, dphi, etc. data
  assembly(tid).reinitLowerDElemRef(elem, pts, weights);

  // Actually get the dof indices in the moose variables
  systemBaseNonlinear().prepareLowerD(tid);
  systemBaseAuxiliary().prepareLowerD(tid);

  // With the dof indices set in the moose variables, now let's properly size
  // our local residuals/Jacobians
  assembly(tid).prepareLowerD();

  // Let's finally compute our variable values!
  systemBaseNonlinear().reinitLowerD(tid);
  systemBaseAuxiliary().reinitLowerD(tid);
}

void
SubProblem::reinitMortarElem(const Elem * elem, THREAD_ID tid)
{
  assembly(tid).reinitMortarElem(elem);
}

void
SubProblem::addAlgebraicGhostingFunctor(GhostingFunctor & algebraic_gf, bool to_mesh)
{
  EquationSystems & eq = es();
  auto n_sys = eq.n_systems();

  for (MooseIndex(n_sys) i = 0; i < n_sys; ++i)
    eq.get_system(i).get_dof_map().add_algebraic_ghosting_functor(algebraic_gf, to_mesh);
}

void
SubProblem::automaticScaling(bool automatic_scaling)
{
  systemBaseNonlinear().automaticScaling(automatic_scaling);
}

bool
SubProblem::automaticScaling() const
{
  return systemBaseNonlinear().automaticScaling();
}
