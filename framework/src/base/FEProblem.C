/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "FEProblem.h"
#include "Assembly.h"

template<>
InputParameters validParams<FEProblem>()
{
  InputParameters params = validParams<FEProblemBase>();
  return params;
}

FEProblem::FEProblem(const InputParameters & parameters) :
    FEProblemBase(parameters),
    _use_nonlinear(getParam<bool>("use_nonlinear")),
    _nl_sys(_use_nonlinear ? (new NonlinearSystem(*this, "nl0")) : (new MooseEigenSystem(*this, "eigen0")))
{
  _nl = _nl_sys;
  _aux = new AuxiliarySystem(*this, "aux0");

  newAssemblyArray(*_nl_sys);

  initNullSpaceVectors(parameters, *_nl_sys);

  _eq.parameters.set<FEProblem *>("_fe_problem") = this;
}

FEProblem::~FEProblem()
{
  FEProblemBase::deleteAssemblyArray();

  delete _nl;

  delete _aux;
}

void
FEProblem::setInputParametersFEProblem(InputParameters & parameters)
{
  // set _fe_problem
  FEProblemBase::setInputParametersFEProblem(parameters);
  // set _fe_problem
  parameters.set<FEProblem *>("_fe_problem") = this;
<<<<<<< 58cc01c9868219ca6a599245f302976962cc3777
=======
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->auxSys();
  }
  else
  {
    if (_displaced_problem == NULL && parameters.get<bool>("use_displaced_mesh"))
    {
      // We allow AuxScalarKernels to request that they use_displaced_mesh,
      // but then be overridden when no displacements variables are
      // provided in the Mesh block.  If that happened, update the value
      // of use_displaced_mesh appropriately for this AuxScalarKernel.
      if (parameters.have_parameter<bool>("use_displaced_mesh"))
        parameters.set<bool>("use_displaced_mesh") = false;
    }

    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = &_aux;
  }
  _aux.addScalarKernel(kernel_name, name, parameters);
}

void
FEProblem::addDiracKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters)
{
  parameters.set<FEProblem *>("_fe_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->nlSys();
    _reinit_displaced_elem = true;
  }
  else
  {
    if (_displaced_problem == NULL && parameters.get<bool>("use_displaced_mesh"))
    {
      // We allow DiracKernels to request that they use_displaced_mesh,
      // but then be overridden when no displacements variables are
      // provided in the Mesh block.  If that happened, update the value
      // of use_displaced_mesh appropriately for this DiracKernel.
      if (parameters.have_parameter<bool>("use_displaced_mesh"))
        parameters.set<bool>("use_displaced_mesh") = false;
    }

    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = &_nl;
  }
  _nl.addDiracKernel(kernel_name, name, parameters);
}

// DGKernels ////

void
FEProblem::addDGKernel(const std::string & dg_kernel_name, const std::string & name, InputParameters parameters)
{
  parameters.set<FEProblem *>("_fe_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->nlSys();
    _reinit_displaced_face = true;
  }
  else
  {
    if (_displaced_problem == NULL && parameters.get<bool>("use_displaced_mesh"))
    {
      // We allow DGKernels to request that they use_displaced_mesh,
      // but then be overridden when no displacements variables are
      // provided in the Mesh block.  If that happened, update the value
      // of use_displaced_mesh appropriately for this DGKernel.
      if (parameters.have_parameter<bool>("use_displaced_mesh"))
        parameters.set<bool>("use_displaced_mesh") = false;
    }

    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = &_nl;
  }
  _nl.addDGKernel(dg_kernel_name, name, parameters);
}

// InterfaceKernels ////

void
FEProblem::addInterfaceKernel(const std::string & interface_kernel_name, const std::string & name, InputParameters parameters)
{
  parameters.set<FEProblem *>("_fe_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->nlSys();
    _reinit_displaced_face = true;
  }
  else
  {
    if (_displaced_problem == NULL && parameters.get<bool>("use_displaced_mesh"))
    {
      // We allow InterfaceKernels to request that they use_displaced_mesh,
      // but then be overridden when no displacements variables are
      // provided in the Mesh block.  If that happened, update the value
      // of use_displaced_mesh appropriately for this InterfaceKernel.
      if (parameters.have_parameter<bool>("use_displaced_mesh"))
        parameters.set<bool>("use_displaced_mesh") = false;
    }

    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = &_nl;
  }
  _nl.addInterfaceKernel(interface_kernel_name, name, parameters);
}

void
FEProblem::addInitialCondition(const std::string & ic_name, const std::string & name, InputParameters parameters)
{

  // before we start to mess with the initial condition, we need to check parameters for errors.
  parameters.checkParams(name);

  parameters.set<FEProblem *>("_fe_problem") = this;
  parameters.set<SubProblem *>("_subproblem") = this;

  const std::string & var_name = parameters.get<VariableName>("variable");
  // field IC
  if (hasVariable(var_name))
  {
    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
    {
      MooseVariable & var = getVariable(tid, var_name);
      parameters.set<SystemBase *>("_sys") = &var.sys();
      MooseSharedPointer<InitialCondition> ic = _factory.create<InitialCondition>(ic_name, name, parameters, tid);
      _ics.addObject(ic, tid);
    }
  }

  // scalar IC
  else if (hasScalarVariable(var_name))
  {
    MooseVariableScalar & var = getScalarVariable(0, var_name);
    parameters.set<SystemBase *>("_sys") = &var.sys();
    MooseSharedPointer<ScalarInitialCondition> ic = _factory.create<ScalarInitialCondition>(ic_name, name, parameters);
    _scalar_ics.addObject(ic);
  }

  else
    mooseError("Variable '" << var_name << "' requested in initial condition '" << name << "' does not exist.");
}

void
FEProblem::projectSolution()
{
  Moose::perf_log.push("projectSolution()", "Utility");

  Moose::enableFPE();

  ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();
  ComputeInitialConditionThread cic(*this);
  Threads::parallel_reduce(elem_range, cic);

  // Need to close the solution vector here so that boundary ICs take precendence
  _nl.solution().close();
  _aux.solution().close();

  // now run boundary-restricted initial conditions
  ConstBndNodeRange & bnd_nodes = *_mesh.getBoundaryNodeRange();
  ComputeBoundaryInitialConditionThread cbic(*this);
  Threads::parallel_reduce(bnd_nodes, cbic);

  _nl.solution().close();
  _aux.solution().close();

  // Also, load values into the SCALAR dofs
  // Note: We assume that all SCALAR dofs are on the
  // processor with highest ID
  if (processor_id() == (n_processors()-1) && _scalar_ics.hasActiveObjects())
  {
    const std::vector<MooseSharedPointer<ScalarInitialCondition> > & ics = _scalar_ics.getActiveObjects();
    for (const auto & ic : ics)
    {
      MooseVariableScalar & var = ic->variable();
      var.reinit();

      DenseVector<Number> vals(var.order());
      ic->compute(vals);

      const unsigned int n_SCALAR_dofs = var.dofIndices().size();
      for (unsigned int i = 0; i < n_SCALAR_dofs; i++)
      {
        const dof_id_type global_index = var.dofIndices()[i];
        var.sys().solution().set(global_index, vals(i));
        var.setValue(i, vals(i));
      }
    }
  }

  Moose::enableFPE(false);

  _nl.solution().close();
  _nl.solution().localize(*_nl.sys().current_local_solution, _nl.dofMap().get_send_list());

  _aux.solution().close();
  _aux.solution().localize(*_aux.sys().current_local_solution, _aux.dofMap().get_send_list());

  Moose::perf_log.pop("projectSolution()", "Utility");
}


MooseSharedPointer<Material>
FEProblem::getMaterial(std::string name, Moose::MaterialDataType type, THREAD_ID tid)
{
  switch (type)
  {
  case Moose::NEIGHBOR_MATERIAL_DATA:
    name += "_neighbor";
    break;
  case Moose::FACE_MATERIAL_DATA:
    name += "_face";
    break;
  default:
    break;
  }

  MooseSharedPointer<Material> material = _all_materials[type].getActiveObject(name, tid);
  if (material->getParam<bool>("compute") && type == Moose::BLOCK_MATERIAL_DATA)
    mooseWarning("You are retrieving a Material object (" << material->name() << "), but its compute flag is not set to true. This indicates that MOOSE is computing this property which may not be desired and produce un-expected results.");

  return material;
}


MooseSharedPointer<MaterialData>
FEProblem::getMaterialData(Moose::MaterialDataType type, THREAD_ID tid)
{
  MooseSharedPointer<MaterialData> output;
  switch (type)
  {
  case Moose::BLOCK_MATERIAL_DATA:
    output = _material_data[tid];
    break;
  case Moose::NEIGHBOR_MATERIAL_DATA:
    output = _neighbor_material_data[tid];
    break;
  case Moose::BOUNDARY_MATERIAL_DATA:
  case Moose::FACE_MATERIAL_DATA:
    output = _bnd_material_data[tid];
    break;
  }
  return output;
}


void
FEProblem::addMaterial(const std::string & mat_name, const std::string & name, InputParameters parameters)
{
  parameters.set<FEProblem *>("_fe_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    _reinit_displaced_elem = true;
  }
  else
  {
    if (_displaced_problem == NULL && parameters.get<bool>("use_displaced_mesh"))
    {
      // We allow Materials to request that they use_displaced_mesh,
      // but then be overridden when no displacements variables are
      // provided in the Mesh block.  If that happened, update the value
      // of use_displaced_mesh appropriately for this Material.
      if (parameters.have_parameter<bool>("use_displaced_mesh"))
        parameters.set<bool>("use_displaced_mesh") = false;
    }

    parameters.set<SubProblem *>("_subproblem") = this;
  }

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    // Create the general Block/Boundary Material object
    MooseSharedPointer<Material> material = _factory.create<Material>(mat_name, name, parameters, tid);
    bool discrete = !material->getParam<bool>("compute");

    // If the object is boundary restricted do not create the nieghbor and face objects
    if (material->boundaryRestricted())
    {
      _all_materials.addObject(material, tid);
      if (discrete)
        _discrete_materials.addObject(material, tid);
      else
        _materials.addObject(material, tid);
    }

    // Non-boundary restricted require face and neighbor objects
    else
    {
      // The name of the object being created, this is changed multiple times as objects are created below
      std::string object_name;

      // Create a copy of the supplied parameters to the setting for "_material_data_type" isn't used from a previous tid loop
      InputParameters current_parameters = parameters;

      // face material
      current_parameters.set<Moose::MaterialDataType>("_material_data_type") = Moose::FACE_MATERIAL_DATA;
      object_name = name + "_face";
      MooseSharedPointer<Material> face_material = _factory.create<Material>(mat_name, object_name, current_parameters, tid);

      // neighbor material
      current_parameters.set<Moose::MaterialDataType>("_material_data_type") = Moose::NEIGHBOR_MATERIAL_DATA;
      current_parameters.set<bool>("_neighbor") = true;
      object_name = name + "_neighbor";
      MooseSharedPointer<Material> neighbor_material = _factory.create<Material>(mat_name, object_name, current_parameters, tid);

      // Store the material objects
      _all_materials.addObjects(material, neighbor_material, face_material, tid);

      if (discrete)
        _discrete_materials.addObjects(material, neighbor_material, face_material, tid);
      else
        _materials.addObjects(material, neighbor_material, face_material, tid);

        // link enabled parameter of face and neighbor materials
      MooseObjectParameterName name(MooseObjectName("Material", material->name()), "enabled");
      MooseObjectParameterName face_name(MooseObjectName("Material", face_material->name()), "enabled");
      MooseObjectParameterName neighbor_name(MooseObjectName("Material", neighbor_material->name()), "enabled");
      _app.getInputParameterWarehouse().addControllableParameterConnection(name, face_name);
      _app.getInputParameterWarehouse().addControllableParameterConnection(name, neighbor_name);

    }
  }
}


void
FEProblem::prepareMaterials(SubdomainID blk_id, THREAD_ID tid)
{
  std::set<MooseVariable *> needed_moose_vars;

  if (_all_materials.hasActiveBlockObjects(blk_id, tid))
    _all_materials.updateVariableDependency(needed_moose_vars, tid);

  const std::set<BoundaryID> & ids = _mesh.getSubdomainBoundaryIds(blk_id);
  for (const auto & id : ids)
    _materials.updateBoundaryVariableDependency(id, needed_moose_vars, tid);

  const std::set<MooseVariable *> & current_active_elemental_moose_variables = getActiveElementalMooseVariables(tid);
  needed_moose_vars.insert(current_active_elemental_moose_variables.begin(), current_active_elemental_moose_variables.end());

  if (!needed_moose_vars.empty())
    setActiveElementalMooseVariables(needed_moose_vars, tid);
}

void
FEProblem::reinitMaterials(SubdomainID blk_id, THREAD_ID tid, bool swap_stateful)
{
  if (_all_materials.hasActiveBlockObjects(blk_id, tid))
  {
    const Elem * & elem = _assembly[tid]->elem();
    unsigned int n_points = _assembly[tid]->qRule()->n_points();
    if (_material_data[tid]->nQPoints() != n_points)
      _material_data[tid]->size(n_points);

    // Only swap if requested
    if (swap_stateful)
      _material_data[tid]->swap(*elem);

    if (_discrete_materials.hasActiveBlockObjects(blk_id, tid))
      _material_data[tid]->reset(_discrete_materials.getActiveBlockObjects(blk_id, tid));

    if (_materials.hasActiveBlockObjects(blk_id, tid))
      _material_data[tid]->reinit(_materials.getActiveBlockObjects(blk_id, tid));
  }
}

void
FEProblem::reinitMaterialsFace(SubdomainID blk_id, THREAD_ID tid, bool swap_stateful)
{
  if (_all_materials[Moose::FACE_MATERIAL_DATA].hasActiveBlockObjects(blk_id, tid))
  {
    const Elem * & elem = _assembly[tid]->elem();
    unsigned int side = _assembly[tid]->side();
    unsigned int n_points = _assembly[tid]->qRuleFace()->n_points();

    if (_bnd_material_data[tid]->nQPoints() != n_points)
      _bnd_material_data[tid]->size(n_points);

    if (swap_stateful && !_bnd_material_data[tid]->isSwapped())
      _bnd_material_data[tid]->swap(*elem, side);

    if (_discrete_materials[Moose::FACE_MATERIAL_DATA].hasActiveBlockObjects(blk_id, tid))
      _bnd_material_data[tid]->reset(_discrete_materials[Moose::FACE_MATERIAL_DATA].getActiveBlockObjects(blk_id, tid));

    if (_materials[Moose::FACE_MATERIAL_DATA].hasActiveBlockObjects(blk_id, tid))
      _bnd_material_data[tid]->reinit(_materials[Moose::FACE_MATERIAL_DATA].getActiveBlockObjects(blk_id, tid));
  }
}

void
FEProblem::reinitMaterialsNeighbor(SubdomainID blk_id, THREAD_ID tid, bool swap_stateful)
{
  if (_all_materials[Moose::NEIGHBOR_MATERIAL_DATA].hasActiveBlockObjects(blk_id, tid))
  {
    // NOTE: this will not work with h-adaptivity
    const Elem * & neighbor = _assembly[tid]->neighbor();
    unsigned int neighbor_side = neighbor->which_neighbor_am_i(_assembly[tid]->elem());
    unsigned int n_points = _assembly[tid]->qRuleFace()->n_points();
    if (_neighbor_material_data[tid]->nQPoints() != n_points)
      _neighbor_material_data[tid]->size(n_points);

    // Only swap if requested
    if (swap_stateful)
      _neighbor_material_data[tid]->swap(*neighbor, neighbor_side);

    if (_discrete_materials[Moose::NEIGHBOR_MATERIAL_DATA].hasActiveBlockObjects(blk_id, tid))
      _neighbor_material_data[tid]->reset(_discrete_materials[Moose::NEIGHBOR_MATERIAL_DATA].getActiveBlockObjects(blk_id, tid));

    if (_materials[Moose::NEIGHBOR_MATERIAL_DATA].hasActiveBlockObjects(blk_id, tid))
      _neighbor_material_data[tid]->reinit(_materials[Moose::NEIGHBOR_MATERIAL_DATA].getActiveBlockObjects(blk_id, tid));
  }
}

void
FEProblem::reinitMaterialsBoundary(BoundaryID boundary_id, THREAD_ID tid, bool swap_stateful)
{
  if (_all_materials.hasActiveBoundaryObjects(boundary_id, tid))
  {
    const Elem * & elem = _assembly[tid]->elem();
    unsigned int side = _assembly[tid]->side();
    unsigned int n_points = _assembly[tid]->qRuleFace()->n_points();
    if (_bnd_material_data[tid]->nQPoints() != n_points)
      _bnd_material_data[tid]->size(n_points);

    if (swap_stateful && !_bnd_material_data[tid]->isSwapped())
      _bnd_material_data[tid]->swap(*elem, side);

    if (_discrete_materials.hasActiveBoundaryObjects(boundary_id, tid))
      _bnd_material_data[tid]->reset(_discrete_materials.getActiveBoundaryObjects(boundary_id, tid));

    if (_materials.hasActiveBoundaryObjects(boundary_id, tid))
      _bnd_material_data[tid]->reinit(_materials.getActiveBoundaryObjects(boundary_id, tid));
  }
}

void
FEProblem::swapBackMaterials(THREAD_ID tid)
{
  const Elem * & elem = _assembly[tid]->elem();
  _material_data[tid]->swapBack(*elem);
}

void
FEProblem::swapBackMaterialsFace(THREAD_ID tid)
{
  const Elem * & elem = _assembly[tid]->elem();
  unsigned int side = _assembly[tid]->side();
  _bnd_material_data[tid]->swapBack(*elem, side);
}

void
FEProblem::swapBackMaterialsNeighbor(THREAD_ID tid)
{
  // NOTE: this will not work with h-adaptivity
  const Elem * & neighbor = _assembly[tid]->neighbor();
  unsigned int neighbor_side = neighbor->which_neighbor_am_i(_assembly[tid]->elem());
  _neighbor_material_data[tid]->swapBack(*neighbor, neighbor_side);
}

/**
 * Small helper function used by addPostprocessor to try to get a Postprocessor pointer from a MooseObject
 */
MooseSharedPointer<Postprocessor>
getPostprocessorPointer(MooseSharedPointer<MooseObject> mo)
{
  {
    MooseSharedPointer<ElementPostprocessor> intermediate = MooseSharedNamespace::dynamic_pointer_cast<ElementPostprocessor>(mo);
    if (intermediate.get())
      return MooseSharedNamespace::static_pointer_cast<Postprocessor>(intermediate);
  }

  {
    MooseSharedPointer<NodalPostprocessor> intermediate = MooseSharedNamespace::dynamic_pointer_cast<NodalPostprocessor>(mo);
    if (intermediate.get())
      return MooseSharedNamespace::static_pointer_cast<Postprocessor>(intermediate);
  }

  {
    MooseSharedPointer<InternalSidePostprocessor> intermediate = MooseSharedNamespace::dynamic_pointer_cast<InternalSidePostprocessor>(mo);
    if (intermediate.get())
      return MooseSharedNamespace::static_pointer_cast<Postprocessor>(intermediate);
  }

  {
    MooseSharedPointer<SidePostprocessor> intermediate = MooseSharedNamespace::dynamic_pointer_cast<SidePostprocessor>(mo);
    if (intermediate.get())
      return MooseSharedNamespace::static_pointer_cast<Postprocessor>(intermediate);
  }

  {
    MooseSharedPointer<GeneralPostprocessor> intermediate = MooseSharedNamespace::dynamic_pointer_cast<GeneralPostprocessor>(mo);
    if (intermediate.get())
      return MooseSharedNamespace::static_pointer_cast<Postprocessor>(intermediate);
  }

  return MooseSharedPointer<Postprocessor>();
}

template <typename UO_TYPE, typename PP_TYPE>
Postprocessor *
getPostprocessorPointer(UO_TYPE * uo)
{
  PP_TYPE * intermediate = dynamic_cast<PP_TYPE *>(uo);
  if (intermediate)
    return static_cast<Postprocessor *>(intermediate);

  return NULL;
}


void
FEProblem::initPostprocessorData(const std::string & name)
{
  _pps_data.init(name);
}

void
FEProblem::addPostprocessor(std::string pp_name, const std::string & name, InputParameters parameters)
{
  // Check for name collision
  if (_all_user_objects.hasActiveObject(name))
    mooseError(std::string("A UserObject with the name \"") + name + "\" already exists.  You may not add a Postprocessor by the same name.");

  addUserObject(pp_name, name, parameters);
  initPostprocessorData(name);
}

void
FEProblem::addVectorPostprocessor(std::string pp_name, const std::string & name, InputParameters parameters)
{
  // Check for name collision
  if (_all_user_objects.hasActiveObject(name))
    mooseError(std::string("A UserObject with the name \"") + name + "\" already exists.  You may not add a VectorPostprocessor by the same name.");

  addUserObject(pp_name, name, parameters);
}

void
FEProblem::addUserObject(std::string user_object_name, const std::string & name, InputParameters parameters)
{
  parameters.set<FEProblem *>("_fe_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
  else
  {
    if (_displaced_problem == NULL && parameters.get<bool>("use_displaced_mesh"))
    {
      // We allow UserObjects to request that they use_displaced_mesh,
      // but then be overridden when no displacements variables are
      // provided in the Mesh block.  If that happened, update the value
      // of use_displaced_mesh appropriately for this UserObject.
      if (parameters.have_parameter<bool>("use_displaced_mesh"))
        parameters.set<bool>("use_displaced_mesh") = false;
    }

    parameters.set<SubProblem *>("_subproblem") = this;
  }

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
  {
    // Create the UserObject
    MooseSharedPointer<UserObject> user_object = _factory.create<UserObject>(user_object_name, name, parameters, tid);
    _all_user_objects.addObject(user_object, tid);

    // Attempt to create all the possible UserObject types
    MooseSharedPointer<ElementUserObject> euo = MooseSharedNamespace::dynamic_pointer_cast<ElementUserObject>(user_object);
    MooseSharedPointer<SideUserObject> suo = MooseSharedNamespace::dynamic_pointer_cast<SideUserObject>(user_object);
    MooseSharedPointer<InternalSideUserObject> isuo = MooseSharedNamespace::dynamic_pointer_cast<InternalSideUserObject>(user_object);
    MooseSharedPointer<NodalUserObject> nuo = MooseSharedNamespace::dynamic_pointer_cast<NodalUserObject>(user_object);
    MooseSharedPointer<GeneralUserObject> guo = MooseSharedNamespace::dynamic_pointer_cast<GeneralUserObject>(user_object);

    // Account for displaced mesh use
    if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
    {
      if (euo || nuo)
        _reinit_displaced_elem = true;
      else if (suo)
        _reinit_displaced_face = true;
    }

    // Add the object to the correct warehouse
    if (guo)
    {
      _general_user_objects.addObject(guo);
      break; // not threaded
    }
    else if (nuo)
      _nodal_user_objects.addObject(nuo, tid);
    else if (suo)
      _side_user_objects.addObject(suo, tid);
    else if (isuo)
      _internal_side_user_objects.addObject(isuo, tid);
    else if (euo)
      _elemental_user_objects.addObject(euo, tid);
  }
}

const UserObject &
FEProblem::getUserObjectBase(const std::string & name)
{
  if (_all_user_objects.hasActiveObject(name))
    return *(_all_user_objects.getActiveObject(name).get());

  mooseError("Unable to find user object with name '" + name + "'");
}

bool
FEProblem::hasUserObject(const std::string & name)
{
  return _all_user_objects.hasActiveObject(name);
}

bool
FEProblem::hasPostprocessor(const std::string & name)
{
  return _pps_data.hasPostprocessor(name);
}

PostprocessorValue &
FEProblem::getPostprocessorValue(const PostprocessorName & name)
{
  return _pps_data.getPostprocessorValue(name);
}

PostprocessorValue &
FEProblem::getPostprocessorValueOld(const std::string & name)
{
  return _pps_data.getPostprocessorValueOld(name);
}

PostprocessorValue &
FEProblem::getPostprocessorValueOlder(const std::string & name)
{
  return _pps_data.getPostprocessorValueOlder(name);
}

VectorPostprocessorData &
FEProblem::getVectorPostprocessorData()
{
  return _vpps_data;
}

bool
FEProblem::hasVectorPostprocessor(const std::string & name)
{
  return _vpps_data.hasVectorPostprocessor(name);
}

VectorPostprocessorValue &
FEProblem::getVectorPostprocessorValue(const VectorPostprocessorName & name, const std::string & vector_name)
{
  return _vpps_data.getVectorPostprocessorValue(name, vector_name);
}

VectorPostprocessorValue &
FEProblem::getVectorPostprocessorValueOld(const std::string & name, const std::string & vector_name)
{
  return _vpps_data.getVectorPostprocessorValueOld(name, vector_name);
}

VectorPostprocessorValue &
FEProblem::declareVectorPostprocessorVector(const VectorPostprocessorName & name, const std::string & vector_name)
{
  return _vpps_data.declareVector(name, vector_name);
}

const std::map<std::string, VectorPostprocessorData::VectorPostprocessorState> &
FEProblem::getVectorPostprocessorVectors(const std::string & vpp_name)
{
  return _vpps_data.vectors(vpp_name);
}

void
FEProblem::parentOutputPositionChanged()
{
  for (const auto & it : _multi_apps)
  {
    const std::vector<MooseSharedPointer<MultiApp> > & objects = it.second.getActiveObjects();
    for (const auto & obj : objects)
      obj->parentOutputPositionChanged();
  }
}

void
FEProblem::computeIndicatorsAndMarkers()
{
  computeIndicators();
  computeMarkers();
}

void
FEProblem::computeIndicators()
{
  // Initialize indicator aux variable fields
  if (_indicators.hasActiveObjects() || _internal_side_indicators.hasActiveObjects())
  {
    std::vector<std::string> fields;

    // Indicator Fields
    const std::vector<MooseSharedPointer<Indicator> > & indicators = _indicators.getActiveObjects();
    for (const auto & indicator : indicators)
      fields.push_back(indicator->name());

    // InternalSideIndicator Fields
    const std::vector<MooseSharedPointer<InternalSideIndicator> > & internal_indicators = _internal_side_indicators.getActiveObjects();
    for (const auto & internal_indicator : internal_indicators)
      fields.push_back(internal_indicator->name());

    _aux.zeroVariables(fields);
  }

  // compute Indicators
  if (_indicators.hasActiveObjects() || _internal_side_indicators.hasActiveObjects())
  {
    ComputeIndicatorThread cit(*this);
    Threads::parallel_reduce(*_mesh.getActiveLocalElementRange(), cit);
    _aux.solution().close();
    _aux.update();

    ComputeIndicatorThread finalize_cit(*this, true);
    Threads::parallel_reduce(*_mesh.getActiveLocalElementRange(), finalize_cit);
    _aux.solution().close();
    _aux.update();
  }
}

void
FEProblem::computeMarkers()
{
  // Initialize marker aux variable fields
  if (_markers.hasActiveObjects())
  {
    std::vector<std::string> fields;

    // Marker Fields
    const std::vector<MooseSharedPointer<Marker> > & markers = _markers.getActiveObjects();
    for (const auto & marker : markers)
      fields.push_back(marker->name());

    _aux.zeroVariables(fields);
  }

  // compute Markers
  if (_markers.hasActiveObjects())
  {
    _adaptivity.updateErrorVectors();

    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
    {
      const std::vector<MooseSharedPointer<Marker> > & markers = _markers.getActiveObjects(tid);
      for (const auto & marker : markers)
        marker->markerSetup();
    }

    ComputeMarkerThread cmt(*this);
    Threads::parallel_reduce(*_mesh.getActiveLocalElementRange(), cmt);

    _aux.solution().close();
    _aux.update();
  }
}

const ExecFlagType &
FEProblem::getCurrentExecuteOnFlag() const
{
  return _current_execute_on_flag;
}


void
FEProblem::execute(const ExecFlagType & exec_type)
{
  // Set the current flag
  _current_execute_on_flag = exec_type;
  if (exec_type == EXEC_NONLINEAR)
    _currently_computing_jacobian = true;


  // Pre-aux UserObjects
  Moose::perf_log.push("computeUserObjects()", "Execution");
  computeUserObjects(exec_type, Moose::PRE_AUX);
  Moose::perf_log.pop("computeUserObjects()", "Execution");

  // AuxKernels
  Moose::perf_log.push("computeAuxiliaryKernels()", "Execution");
  computeAuxiliaryKernels(exec_type);
  Moose::perf_log.pop("computeAuxiliaryKernels()", "Execution");

  // Post-aux UserObjects
  Moose::perf_log.push("computeUserObjects()", "Execution");
  computeUserObjects(exec_type, Moose::POST_AUX);
  Moose::perf_log.pop("computeUserObjects()", "Execution");

  // Controls
  Moose::perf_log.push("computeControls()", "Execution");
  executeControls(exec_type);
  Moose::perf_log.pop("computeControls()", "Execution");

  // Return the current flag to None
  _current_execute_on_flag = EXEC_NONE;
  _currently_computing_jacobian = false;
}

void
FEProblem::computeAuxiliaryKernels(const ExecFlagType & type)
{
  _aux.compute(type);
}

void
FEProblem::computeUserObjects(const ExecFlagType & type, const Moose::AuxGroup & group)
{
  // Get convenience reference to active warehouse
  const MooseObjectWarehouse<ElementUserObject> & elemental = _elemental_user_objects[group][type];
  const MooseObjectWarehouse<SideUserObject> & side = _side_user_objects[group][type];
  const MooseObjectWarehouse<InternalSideUserObject> & internal_side = _internal_side_user_objects[group][type];
  const MooseObjectWarehouse<NodalUserObject> & nodal = _nodal_user_objects[group][type];
  const MooseObjectWarehouse<GeneralUserObject> & general = _general_user_objects[group][type];

  // Perform Residual/Jacobian setups
  switch (type)
  {
  case EXEC_LINEAR:
    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
    {
      elemental.residualSetup(tid);
      side.residualSetup(tid);
      internal_side.residualSetup(tid);
      nodal.residualSetup(tid);
    }
    general.residualSetup();
    break;

  case EXEC_NONLINEAR:
    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
    {
      elemental.jacobianSetup(tid);
      side.jacobianSetup(tid);
      internal_side.jacobianSetup(tid);
      nodal.jacobianSetup(tid);
    }
    general.jacobianSetup();
    break;

  default:
    break;
  }

  // Legacy AuxKernel computation
  if (_elemental_user_objects[Moose::ALL][type].hasActiveBlockObjects() ||
      _internal_side_user_objects[Moose::ALL][type].hasActiveBlockObjects() ||
      _side_user_objects[Moose::ALL][type].hasActiveBoundaryObjects() ||
      _internal_side_user_objects[Moose::ALL][type].hasActiveObjects() ||
      _nodal_user_objects[Moose::ALL][type].hasActiveBlockObjects() )
  {
    serializeSolution();
    if (_displaced_problem != NULL)
    _displaced_problem->updateMesh();

    if (_use_legacy_uo_aux_computation)
        _aux.compute(EXEC_LINEAR);
  }

  // Initialize Elemental/Side/InternalSideUserObjects
  initializeUserObjects<ElementUserObject>(elemental);
  initializeUserObjects<SideUserObject>(side);
  initializeUserObjects<InternalSideUserObject>(internal_side);

  // Execute Elemental/Side/InternalSideUserObjects
  if (elemental.hasActiveObjects() || side.hasActiveObjects() || internal_side.hasActiveObjects())
  {
    ComputeUserObjectsThread cppt(*this, getNonlinearSystem(), elemental, side, internal_side);
    Threads::parallel_reduce(*_mesh.getActiveLocalElementRange(), cppt);
  }

  // Finalize, threadJoin, and update PP values of Elemental/Side/InternalSideUserObjects
  finalizeUserObjects<SideUserObject>(side);
  finalizeUserObjects<InternalSideUserObject>(internal_side);
  finalizeUserObjects<ElementUserObject>(elemental);

  // Initialize Nodal
  initializeUserObjects<NodalUserObject>(nodal);

  // Execute NodalUserObjects
  if (nodal.hasActiveObjects())
  {
    ComputeNodalUserObjectsThread cnppt(*this, nodal);
    Threads::parallel_reduce(*_mesh.getLocalNodeRange(), cnppt);
  }

  // Finalize, threadJoin, and update PP values of Nodal
  finalizeUserObjects<NodalUserObject>(nodal);

  // Execute GeneralUserObjects
  if (general.hasActiveObjects())
  {
    std::set<MooseVariable *> needed_moose_vars;
    general.updateVariableDependency(needed_moose_vars, 0);
    setActiveElementalMooseVariables(needed_moose_vars, 0);

    const std::vector<MooseSharedPointer<GeneralUserObject> > & objects = general.getActiveObjects();
    for (const auto & obj : objects)
    {
      obj->initialize();
      obj->execute();
      obj->finalize();

      MooseSharedPointer<Postprocessor> pp = MooseSharedNamespace::dynamic_pointer_cast<Postprocessor>(obj);
      if (pp)
        _pps_data.storeValue(obj->name(), pp->getValue());
    }
  }
}

void
FEProblem::executeControls(const ExecFlagType & exec_type)
{
  _control_warehouse.setup(exec_type);
  const std::vector<MooseSharedPointer<Control> > & objects = _control_warehouse[exec_type].getActiveObjects();
  for (const auto & control : objects)
    control->execute();
}

void
FEProblem::updateActiveObjects()
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
  {
    _nl.updateActive(tid);
    _aux.updateActive(tid);
    _indicators.updateActive(tid);
    _internal_side_indicators.updateActive(tid);
    _markers.updateActive(tid);
    _all_materials.updateActive(tid);
    _materials.updateActive(tid);
    _discrete_materials.updateActive(tid);
    _nodal_user_objects.updateActive(tid);
    _elemental_user_objects.updateActive(tid);
    _side_user_objects.updateActive(tid);
    _internal_side_user_objects.updateActive(tid);
  }

  _general_user_objects.updateActive();
  _control_warehouse.updateActive();
  _multi_apps.updateActive();
  _transient_multi_apps.updateActive();
  _transfers.updateActive();
  _to_multi_app_transfers.updateActive();
  _from_multi_app_transfers.updateActive();
}

void
FEProblem::reportMooseObjectDependency(MooseObject * /*a*/, MooseObject * /*b*/)
{
  //<< "Object " << a->name() << " -> " << b->name() << std::endl;
}

void
FEProblem::reinitBecauseOfGhostingOrNewGeomObjects()
{
  // Need to see if _any_ processor has ghosted elems or geometry objects.
  bool needs_reinit = ! _ghosted_elems.empty();
  needs_reinit = needs_reinit || ! _geometric_search_data._nearest_node_locators.empty();
  needs_reinit = needs_reinit || ( _displaced_problem && ! _displaced_problem->geomSearchData()._nearest_node_locators.empty() );
  _communicator.max(needs_reinit);

  if (needs_reinit)
  {
    // Call reinit to get the ghosted vectors correct now that some geometric search has been done
    _eq.reinit();

    if (_displaced_mesh)
      _displaced_problem->es().reinit();
  }
}

void
FEProblem::addDamper(std::string damper_name, const std::string & name, InputParameters parameters)
{
  parameters.set<FEProblem *>("_fe_problem") = this;
  parameters.set<SubProblem *>("_subproblem") = this;
  parameters.set<SystemBase *>("_sys") = &_nl;

  _has_dampers = true;
  _nl.addDamper(damper_name, name, parameters);
}

void
FEProblem::setupDampers()
{
  _nl.setupDampers();
}

void
FEProblem::addIndicator(std::string indicator_name, const std::string & name, InputParameters parameters)
{
  parameters.set<FEProblem *>("_fe_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->auxSys();
    _reinit_displaced_elem = true;
  }
  else
  {
    if (_displaced_problem == NULL && parameters.get<bool>("use_displaced_mesh"))
    {
      // We allow Indicators to request that they use_displaced_mesh,
      // but then be overridden when no displacements variables are
      // provided in the Mesh block.  If that happened, update the value
      // of use_displaced_mesh appropriately for this Indicator.
      if (parameters.have_parameter<bool>("use_displaced_mesh"))
        parameters.set<bool>("use_displaced_mesh") = false;
    }

    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = &_aux;
  }

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    MooseSharedPointer<Indicator> indicator = _factory.create<Indicator>(indicator_name, name, parameters, tid);

    MooseSharedPointer<InternalSideIndicator> isi = MooseSharedNamespace::dynamic_pointer_cast<InternalSideIndicator>(indicator);
    if (isi)
      _internal_side_indicators.addObject(isi, tid);
    else
      _indicators.addObject(indicator, tid);

  }
}

void
FEProblem::addMarker(std::string marker_name, const std::string & name, InputParameters parameters)
{
  parameters.set<FEProblem *>("_fe_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->auxSys();
    _reinit_displaced_elem = true;
  }
  else
  {
    if (_displaced_problem == NULL && parameters.get<bool>("use_displaced_mesh"))
    {
      // We allow Markers to request that they use_displaced_mesh,
      // but then be overridden when no displacements variables are
      // provided in the Mesh block.  If that happened, update the value
      // of use_displaced_mesh appropriately for this Marker.
      if (parameters.have_parameter<bool>("use_displaced_mesh"))
        parameters.set<bool>("use_displaced_mesh") = false;
    }

    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = &_aux;
  }

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    MooseSharedPointer<Marker> marker = _factory.create<Marker>(marker_name, name, parameters, tid);
    _markers.addObject(marker, tid);
  }
}

void
FEProblem::addMultiApp(const std::string & multi_app_name, const std::string & name, InputParameters parameters)
{
  parameters.set<FEProblem *>("_fe_problem") = this;
  parameters.set<MPI_Comm>("_mpi_comm") = _communicator.get();
  parameters.set<MooseSharedPointer<CommandLine> >("_command_line") = _app.commandLine();

  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->auxSys();
    _reinit_displaced_elem = true;
  }
  else
  {
    if (_displaced_problem == NULL && parameters.get<bool>("use_displaced_mesh"))
    {
      // We allow MultiApps to request that they use_displaced_mesh,
      // but then be overridden when no displacements variables are
      // provided in the Mesh block.  If that happened, update the value
      // of use_displaced_mesh appropriately for this MultiApp.
      if (parameters.have_parameter<bool>("use_displaced_mesh"))
        parameters.set<bool>("use_displaced_mesh") = false;
    }

    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = &_aux;
  }

  MooseSharedPointer<MultiApp> multi_app = _factory.create<MultiApp>(multi_app_name, name, parameters);

  _multi_apps.addObject(multi_app);

  // Store TranseintMultiApp objects in another container, this is needed for calling computeDT
  MooseSharedPointer<TransientMultiApp> trans_multi_app = MooseSharedNamespace::dynamic_pointer_cast<TransientMultiApp>(multi_app);
  if (trans_multi_app)
    _transient_multi_apps.addObject(trans_multi_app);
}

bool
FEProblem::hasMultiApp(const std::string & multi_app_name)
{
  return _multi_apps.hasActiveObject(multi_app_name);
}


MooseSharedPointer<MultiApp>
FEProblem::getMultiApp(const std::string & multi_app_name)
{
  return _multi_apps.getActiveObject(multi_app_name);
}

bool
FEProblem::execMultiApps(ExecFlagType type, bool auto_advance)
{
  // Active MultiApps
  const std::vector<MooseSharedPointer<MultiApp> > & multi_apps = _multi_apps[type].getActiveObjects();

  // Do anything that needs to be done to Apps before transfers
  for (const auto & multi_app : multi_apps)
    multi_app->preTransfer(_dt, _time);

  // Execute Transfers _to_ MultiApps
  if (_to_multi_app_transfers[type].hasActiveObjects())
  {
    const std::vector<MooseSharedPointer<Transfer> > & transfers = _to_multi_app_transfers[type].getActiveObjects();

    std::set<MooseVariable *> needed_moose_vars;
    _to_multi_app_transfers[type].updateVariableDependency(needed_moose_vars);
    setActiveElementalMooseVariables(needed_moose_vars, 0);

    _console << COLOR_CYAN << "\nStarting Transfers on " <<  Moose::stringify(type) << " To MultiApps" << COLOR_DEFAULT << std::endl;
    for (const auto & transfer : transfers)
    {
      Moose::perf_log.push(transfer->name(), "Transfers");
      transfer->execute();
      Moose::perf_log.pop(transfer->name(), "Transfers");
    }

    _console << "Waiting For Transfers To Finish" << '\n';
    MooseUtils::parallelBarrierNotify(_communicator);

    _console << COLOR_CYAN << "Transfers on " <<  Moose::stringify(type) << " Are Finished\n" << COLOR_DEFAULT << std::endl;
  }
  else if (multi_apps.size())
    _console << COLOR_CYAN << "\nNo Transfers on " <<  Moose::stringify(type) << " To MultiApps\n" << COLOR_DEFAULT << std::endl;


  // Execute MultiApps
  if (multi_apps.size())
  {
    _console << COLOR_CYAN << "\nExecuting MultiApps on " <<  Moose::stringify(type) << COLOR_DEFAULT << std::endl;

    bool success = true;

    for (const auto & multi_app : multi_apps)
      success = multi_app->solveStep(_dt, _time, auto_advance);

    _console << "Waiting For Other Processors To Finish" << '\n';
    MooseUtils::parallelBarrierNotify(_communicator);

    _communicator.max(success);

    if (!success)
      return false;

    _console << COLOR_CYAN << "Finished Executing MultiApps on " <<  Moose::stringify(type) << "\n" << COLOR_DEFAULT << std::endl;
  }

  // Execute Transfers _from_ MultiApps
  if (_from_multi_app_transfers[type].hasActiveObjects())
  {
    const std::vector<MooseSharedPointer<Transfer> > & transfers = _from_multi_app_transfers[type].getActiveObjects();

<<<<<<< 283838fde7d550a5543e94f030f1047b0082b571
    _console << COLOR_CYAN << "\nStarting Transfers on " <<  Moose::stringify(type) << " From MultiApps" << COLOR_DEFAULT << std::endl;
=======
    std::set<MooseVariable *> needed_moose_vars;
    _from_multi_app_transfers[type].updateVariableDependency(needed_moose_vars);
    setActiveElementalMooseVariables(needed_moose_vars, 0);


    _console << COLOR_CYAN << "Starting Transfers on " <<  Moose::stringify(type) << " From MultiApps" << COLOR_DEFAULT << std::endl;
>>>>>>> Updates to migrate to complete integration with variable dependency interface
    for (const auto & transfer : transfers)
    {
      Moose::perf_log.push(transfer->name(), "Transfers");
      transfer->execute();
      Moose::perf_log.pop(transfer->name(), "Transfers");
    }

    _console << "Waiting For Transfers To Finish" << '\n';
    MooseUtils::parallelBarrierNotify(_communicator);

    _console << COLOR_CYAN << "Transfers " << Moose::stringify(type) << " Are Finished\n" << COLOR_DEFAULT << std::endl;
  }
  else if (multi_apps.size())
    _console << COLOR_CYAN << "\nNo Transfers on " <<  Moose::stringify(type) << " From MultiApps\n" << COLOR_DEFAULT << std::endl;


  // If we made it here then everything passed
  return true;
}

void
FEProblem::advanceMultiApps(ExecFlagType type)
{
  const std::vector<MooseSharedPointer<MultiApp> > multi_apps = _multi_apps[type].getActiveObjects();

  if (multi_apps.size())
  {
    _console << COLOR_CYAN << "\nAdvancing MultiApps" << COLOR_DEFAULT << std::endl;

    for (const auto & multi_app : multi_apps)
      multi_app->advanceStep();

    _console << "Waiting For Other Processors To Finish" << std::endl;
    MooseUtils::parallelBarrierNotify(_communicator);

    _console << COLOR_CYAN << "Finished Advancing MultiApps\n" << COLOR_DEFAULT << std::endl;
  }
}

void
FEProblem::backupMultiApps(ExecFlagType type)
{
  const std::vector<MooseSharedPointer<MultiApp> > multi_apps = _multi_apps[type].getActiveObjects();

  if (multi_apps.size())
  {
    _console << COLOR_CYAN << "\nBacking Up MultiApps" << COLOR_DEFAULT << std::endl;

    for (const auto & multi_app : multi_apps)
      multi_app->backup();

    _console << "Waiting For Other Processors To Finish" << std::endl;
    MooseUtils::parallelBarrierNotify(_communicator);

    _console << COLOR_CYAN << "Finished Backing Up MultiApps\n" << COLOR_DEFAULT << std::endl;
  }
}

void
FEProblem::restoreMultiApps(ExecFlagType type, bool force)
{
  const std::vector<MooseSharedPointer<MultiApp> > multi_apps = _multi_apps[type].getActiveObjects();

  if (multi_apps.size())
  {
    if (force)
      _console << COLOR_CYAN << "\nRestoring Multiapps because of solve failure!" << COLOR_DEFAULT << std::endl;
    else
      _console << COLOR_CYAN << "\nRestoring MultiApps" << COLOR_DEFAULT << std::endl;

    for (const auto & multi_app : multi_apps)
      if (force || multi_app->needsRestoration())
        multi_app->restore();

    _console << "Waiting For Other Processors To Finish" << std::endl;
    MooseUtils::parallelBarrierNotify(_communicator);

    _console << COLOR_CYAN << "Finished Restoring MultiApps\n" << COLOR_DEFAULT << std::endl;
  }
}

Real
FEProblem::computeMultiAppsDT(ExecFlagType type)
{
  const std::vector<MooseSharedPointer<TransientMultiApp> > & multi_apps = _transient_multi_apps[type].getActiveObjects();

  Real smallest_dt = std::numeric_limits<Real>::max();

  for (const auto & multi_app : multi_apps)
    smallest_dt = std::min(smallest_dt, multi_app->computeDT());

  return smallest_dt;
}


void
FEProblem::execTransfers(ExecFlagType type)
{
  if (_transfers[type].hasActiveObjects())
  {
    const std::vector<MooseSharedPointer<Transfer> > & transfers = _transfers[type].getActiveObjects();
    for (const auto & transfer : transfers)
      transfer->execute();
  }
}


void
FEProblem::addTransfer(const std::string & transfer_name, const std::string & name, InputParameters parameters)
{
  parameters.set<FEProblem *>("_fe_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->auxSys();
    _reinit_displaced_elem = true;
  }
  else
  {
    if (_displaced_problem == NULL && parameters.get<bool>("use_displaced_mesh"))
    {
      // We allow Transfers to request that they use_displaced_mesh,
      // but then be overridden when no displacements variables are
      // provided in the Mesh block.  If that happened, update the value
      // of use_displaced_mesh appropriately for this Transfer.
      if (parameters.have_parameter<bool>("use_displaced_mesh"))
        parameters.set<bool>("use_displaced_mesh") = false;
    }

    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = &_aux;
  }

  // Create the Transfer objects
  MooseSharedPointer<Transfer> transfer = _factory.create<Transfer>(transfer_name, name, parameters);

  // Add MultiAppTransfer object
  MooseSharedPointer<MultiAppTransfer> multi_app_transfer = MooseSharedNamespace::dynamic_pointer_cast<MultiAppTransfer>(transfer);
  if (multi_app_transfer)
  {
    if (multi_app_transfer->direction() == MultiAppTransfer::TO_MULTIAPP)
      _to_multi_app_transfers.addObject(multi_app_transfer);
    else
      _from_multi_app_transfers.addObject(multi_app_transfer);
  }
  else
    _transfers.addObject(transfer);
}

bool
FEProblem::hasVariable(const std::string & var_name)
{
  if (_nl.hasVariable(var_name))
    return true;
  else if (_aux.hasVariable(var_name))
    return true;
  else
    return false;
}

MooseVariable &
FEProblem::getVariable(THREAD_ID tid, const std::string & var_name)
{
  if (_nl.hasVariable(var_name))
    return _nl.getVariable(tid, var_name);
  else if (!_aux.hasVariable(var_name))
    mooseError("Unknown variable " + var_name);

  return _aux.getVariable(tid, var_name);
}

bool
FEProblem::hasScalarVariable(const std::string & var_name)
{
  if (_nl.hasScalarVariable(var_name))
    return true;
  else if (_aux.hasScalarVariable(var_name))
    return true;
  else
    return false;
}

MooseVariableScalar &
FEProblem::getScalarVariable(THREAD_ID tid, const std::string & var_name)
{
  if (_nl.hasScalarVariable(var_name))
    return _nl.getScalarVariable(tid, var_name);
  else if (_aux.hasScalarVariable(var_name))
    return _aux.getScalarVariable(tid, var_name);
  else
    mooseError("Unknown variable " + var_name);
}

void
FEProblem::setActiveElementalMooseVariables(const std::set<MooseVariable *> & moose_vars, THREAD_ID tid)
{
  SubProblem::setActiveElementalMooseVariables(moose_vars, tid);

  if (_displaced_problem)
    _displaced_problem->setActiveElementalMooseVariables(moose_vars, tid);
}

const std::set<MooseVariable *> &
FEProblem::getActiveElementalMooseVariables(THREAD_ID tid)
{
  return SubProblem::getActiveElementalMooseVariables(tid);
}

bool
FEProblem::hasActiveElementalMooseVariables(THREAD_ID tid)
{
  return SubProblem::hasActiveElementalMooseVariables(tid);
}

void
FEProblem::clearActiveElementalMooseVariables(THREAD_ID tid)
{
  SubProblem::clearActiveElementalMooseVariables(tid);

  if (_displaced_problem)
    _displaced_problem->clearActiveElementalMooseVariables(tid);
}

void
FEProblem::createQRules(QuadratureType type, Order order, Order volume_order, Order face_order)
{
  if (order == INVALID_ORDER)
  {
    // automatically determine the integration order
    order = _nl.getMinQuadratureOrder();
    if (order<_aux.getMinQuadratureOrder()) order = _aux.getMinQuadratureOrder();
  }

  if (volume_order == INVALID_ORDER)
    volume_order = order;

  if (face_order == INVALID_ORDER)
    face_order = order;

  for (unsigned int tid = 0; tid < libMesh::n_threads(); ++tid)
    _assembly[tid]->createQRules(type, order, volume_order, face_order);

  if (_displaced_problem)
    _displaced_problem->createQRules(type, order, volume_order, face_order);

  // Find the maximum number of quadrature points
  {
    MaxQpsThread mqt(*this, type, std::max(order, volume_order), face_order);
    Threads::parallel_reduce(*_mesh.getActiveLocalElementRange(), mqt);
    _max_qps = mqt.max();
    _max_shape_funcs = mqt.max_shape_funcs();
  }

  for (unsigned int tid = 0; tid < libMesh::n_threads(); ++tid)
  {
    _zero[tid].resize(getMaxQps(), 0);
    _grad_zero[tid].resize(getMaxQps(), RealGradient(0.));
    _second_zero[tid].resize(getMaxQps(), RealTensor(0.));
    _second_phi_zero[tid].resize(getMaxQps(), std::vector<RealTensor>(getMaxShapeFunctions(), RealTensor(0.)));
  }
}

void
FEProblem::setCoupling(Moose::CouplingType type)
{
  _coupling = type;
}

void
FEProblem::setCouplingMatrix(CouplingMatrix * cm)
{
  _coupling = Moose::COUPLING_CUSTOM;
  delete _cm;
  _cm = cm;
}

void
FEProblem::setNonlocalCouplingMatrix()
{
  unsigned int n_vars = _nl.nVariables();
  _nonlocal_cm.resize(n_vars);
  const std::vector<MooseVariable *> & vars = _nl.getVariables(0);
  const std::vector<MooseSharedPointer<KernelBase> > & nonlocal_kernel = _nonlocal_kernels.getObjects();
  for (const auto & ivar : vars)
    for (const auto & kernel : nonlocal_kernel)
    {
      unsigned int i = ivar->number();
      if (i == kernel->variable().number())
        for (const auto & jvar : vars)
        {
          const auto it = _var_dof_map.find(jvar->name());
          if (it != _var_dof_map.end())
          {
            unsigned int j = jvar->number();
            _nonlocal_cm(i,j) = 1;
          }
        }
    }
}

bool
FEProblem::areCoupled(unsigned int ivar, unsigned int jvar)
{
  return (*_cm)(ivar, jvar);
}

std::vector<std::pair<MooseVariable *, MooseVariable *> > &
FEProblem::couplingEntries(THREAD_ID tid)
{
  return _assembly[tid]->couplingEntries();
}

std::vector<std::pair<MooseVariable *, MooseVariable *> > &
FEProblem::nonlocalCouplingEntries(THREAD_ID tid)
{
  return _assembly[tid]->nonlocalCouplingEntries();
}

void
FEProblem::useFECache(bool fe_cache)
{
  if (fe_cache)
    _console << "\nUtilizing FE Shape Function Caching\n" << std::endl;

  unsigned int n_threads = libMesh::n_threads();

  for (unsigned int i = 0; i < n_threads; ++i)
    _assembly[i]->useFECache(fe_cache); //fe_cache);
}

void
FEProblem::init()
{
  if (_initialized)
    return;

  unsigned int n_vars = _nl.nVariables();
  switch (_coupling)
  {
  case Moose::COUPLING_DIAG:
    _cm = new CouplingMatrix(n_vars);
    for (unsigned int i = 0; i < n_vars; i++)
      for (unsigned int j = 0; j < n_vars; j++)
        (*_cm)(i, j) = (i == j ? 1 : 0);
    break;

  // for full jacobian
  case Moose::COUPLING_FULL:
    _cm = new CouplingMatrix(n_vars);
    for (unsigned int i = 0; i < n_vars; i++)
      for (unsigned int j = 0; j < n_vars; j++)
        (*_cm)(i, j) = 1;
    break;

  case Moose::COUPLING_CUSTOM:
    // do nothing, _cm was already set through couplingMatrix() call
    break;
  }

  _nl.dofMap()._dof_coupling = _cm;
  _nl.dofMap().attach_extra_sparsity_function(&extraSparsity, &_nl);
  _nl.dofMap().attach_extra_send_list_function(&extraSendList, &_nl);
  _aux.dofMap().attach_extra_send_list_function(&extraSendList, &_aux);


  if (_solve && n_vars == 0)
    mooseError("No variables specified in the FEProblem '" << name() << "'.");

  ghostGhostedBoundaries(); // We do this again right here in case new boundaries have been added

  Moose::perf_log.push("eq.init()", "Setup");
  _eq.init();
  Moose::perf_log.pop("eq.init()", "Setup");

  Moose::perf_log.push("FEProblem::init::meshChanged()", "Setup");
  _mesh.meshChanged();
  if (_displaced_problem)
    _displaced_mesh->meshChanged();
  Moose::perf_log.pop("FEProblem::init::meshChanged()", "Setup");

  Moose::perf_log.push("NonlinearSystem::update()", "Setup");
  _nl.update();
  Moose::perf_log.pop("NonlinearSystem::update()", "Setup");

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
    _assembly[tid]->init();

  _nl.init();

  if (_displaced_problem)
    _displaced_problem->init();

  _aux.init();

  _initialized = true;
}

void
FEProblem::solve()
{
  Moose::perf_log.push("solve()", "Execution");

#ifdef LIBMESH_HAVE_PETSC
  Moose::PetscSupport::petscSetOptions(*this); // Make sure the PETSc options are setup for this app
#endif

  Moose::setSolverDefaults(*this);

  // Setup the output system for printing linear/nonlinear iteration information
  initPetscOutput();

  possiblyRebuildGeomSearchPatches();

  // reset flag so that linear solver does not use
  // the old converged reason "DIVERGED_NANORINF", when
  // we throw  an exception and stop solve
  _fail_next_linear_convergence_check = false;

  if (_solve)
    _nl.solve();

  if (_solve)
    _nl.update();

  // sync solutions in displaced problem
  if (_displaced_problem)
    _displaced_problem->syncSolutions();

  Moose::perf_log.pop("solve()", "Execution");
}


void
FEProblem::setException(const std::string & message)
{
  _has_exception = true;
  _exception_message = message;
}


void
FEProblem::checkExceptionAndStopSolve()
{
  // See if any processor had an exception.  If it did, get back the
  // processor that the exception occurred on.
  unsigned int processor_id;

  _communicator.maxloc(_has_exception, processor_id);

  if (_has_exception)
  {
    _communicator.broadcast(_exception_message, processor_id);

    // Print the message
    if (_communicator.rank() == 0)
      Moose::err << _exception_message << std::endl;

    // Stop the solve -- this entails setting
    // SNESSetFunctionDomainError() or directly inserting NaNs in the
    // residual vector to let PETSc >= 3.6 return DIVERGED_NANORINF.
    _nl.stopSolve();

    // We've handled this exception, so we no longer have one.
    _has_exception = false;

    // Force the next linear convergence check to fail.
    _fail_next_linear_convergence_check = true;

    // Repropagate the exception, so it can be caught at a higher level, typically
    // this is NonlinearSystem::computeResidual().
    throw MooseException(_exception_message);
  }
}


bool
FEProblem::converged()
{
  if (_solve)
    return _nl.converged();
  else
    return true;
}

unsigned int
FEProblem::nNonlinearIterations()
{
  return _nl.nNonlinearIterations();
}

unsigned int
FEProblem::nLinearIterations()
{
  return _nl.nLinearIterations();
}

Real
FEProblem::finalNonlinearResidual()
{
  return _nl.finalNonlinearResidual();
}

bool
FEProblem::computingInitialResidual()
{
  return _nl.computingInitialResidual();
}

void
FEProblem::copySolutionsBackwards()
{
  _nl.copySolutionsBackwards();
  _aux.copySolutionsBackwards();
}

void
FEProblem::advanceState()
{
  _nl.copyOldSolutions();
  _aux.copyOldSolutions();

  if ( _displaced_problem != NULL )
  {
    _displaced_problem->nlSys().copyOldSolutions();
    _displaced_problem->auxSys().copyOldSolutions();
  }

  _pps_data.copyValuesBack();

  if (_material_props.hasStatefulProperties())
    _material_props.shift();

  if (_bnd_material_props.hasStatefulProperties())
    _bnd_material_props.shift();
}

void
FEProblem::restoreSolutions()
{
  _nl.restoreSolutions();
  _aux.restoreSolutions();

  if (_displaced_problem != NULL)
    _displaced_problem->updateMesh();
}

void
FEProblem::saveOldSolutions()
{
  _nl.saveOldSolutions();
  _aux.saveOldSolutions();
}

void
FEProblem::restoreOldSolutions()
{
  _nl.restoreOldSolutions();
  _aux.restoreOldSolutions();
}

void
FEProblem::outputStep(ExecFlagType type)
{
  _nl.update();
  _aux.update();
  _app.getOutputWarehouse().outputStep(type);
}

void
FEProblem::allowOutput(bool state)
{
  _app.getOutputWarehouse().allowOutput(state);
}

void
FEProblem::forceOutput()
{
  _app.getOutputWarehouse().forceOutput();
}

void
FEProblem::initPetscOutput()
{
  _app.getOutputWarehouse().solveSetup();
  Moose::PetscSupport::petscSetDefaults(*this);
}


Real
FEProblem::relativeSolutionDifferenceNorm()
{
  if (_solve)
    return _nl.relativeSolutionDifferenceNorm();
  else
    return 0;
}

void
FEProblem::onTimestepBegin()
{
  _nl.onTimestepBegin();
}

void
FEProblem::onTimestepEnd()
{
}

void
FEProblem::addTimeIntegrator(const std::string & type, const std::string & name, InputParameters parameters)
{
  parameters.set<FEProblem *>("_fe_problem") = this;
  parameters.set<SubProblem *>("_subproblem") = this;
  _aux.addTimeIntegrator(type, name + ":aux", parameters);
  _nl.addTimeIntegrator(type, name, parameters);
  _has_time_integrator = true;
}

void
FEProblem::addPredictor(const std::string & type, const std::string & name, InputParameters parameters)
{
  parameters.set<FEProblem *>("_fe_problem") = this;
  parameters.set<SubProblem *>("_subproblem") = this;
  MooseSharedPointer<Predictor> predictor = _factory.create<Predictor>(type, name, parameters);
  _nl.setPredictor(predictor);
}

Real
FEProblem::computeResidualL2Norm()
{
  computeResidualType(*_nl.currentSolution(), *_nl.sys().rhs);

  return _nl.sys().rhs->l2_norm();
}

void
FEProblem::computeResidual(NonlinearImplicitSystem &/*sys*/, const NumericVector<Number> & soln, NumericVector<Number> & residual)
{
  try
  {
    computeResidualType(soln, residual, _kernel_type);
  }
  catch (MooseException & e)
  {
    // If a MooseException propagates all the way to here, it means
    // that it was thrown from a MOOSE system where we do not
    // (currently) properly support the throwing of exceptions, and
    // therefore we have no choice but to error out.  It may be
    // *possible* to handle exceptions from other systems, but in the
    // meantime, we don't want to silently swallow any unhandled
    // exceptions here.
    mooseError("An unhandled MooseException was raised during residual computation.  Please contact the MOOSE team for assistance.");
  }
}

void
FEProblem::computeTransientImplicitResidual(Real time, const NumericVector<Number> & u, const NumericVector<Number> & udot, NumericVector<Number> & residual)
{
  _nl.setSolutionUDot(udot);
  NonlinearImplicitSystem &sys = _nl.sys();
  _time = time;
  computeResidual(sys, u, residual);
}

void
FEProblem::computeResidualType(const NumericVector<Number>& soln, NumericVector<Number>& residual, Moose::KernelType type)
{
  _nl.setSolution(soln);

  _nl.zeroVariablesForResidual();
  _aux.zeroVariablesForResidual();

  unsigned int n_threads = libMesh::n_threads();

  // Random interface objects
  for (const auto & it : _random_data_objects)
    it.second->updateSeeds(EXEC_LINEAR);

  execTransfers(EXEC_LINEAR);

  execMultiApps(EXEC_LINEAR);

  for (unsigned int tid = 0; tid < n_threads; tid++)
    reinitScalars(tid);

  computeUserObjects(EXEC_LINEAR, Moose::PRE_AUX);

  if (_displaced_problem != NULL)
    _displaced_problem->updateMesh();



  for (THREAD_ID tid = 0; tid < n_threads; tid++)
  {
    _all_materials.residualSetup(tid);
    _functions.residualSetup(tid);
  }
  _aux.residualSetup();

  _nl.computeTimeDerivatives();

  try
  {
    _aux.compute(EXEC_LINEAR);
  }
  catch (MooseException & e)
  {
    _console << "\nA MooseException was raised during Auxiliary variable computation.\n"
             << "The next solve will fail, the timestep will be reduced, and we will try again.\n" << std::endl;

    // We know the next solve is going to fail, so there's no point in
    // computing anything else after this.  Plus, using incompletely
    // computed AuxVariables in subsequent calculations could lead to
    // other errors or unhandled exceptions being thrown.
    return;
  }

  computeUserObjects(EXEC_LINEAR, Moose::POST_AUX);

  executeControls(EXEC_LINEAR);

  _app.getOutputWarehouse().residualSetup();

  _nl.computeResidual(residual, type);
}

void
FEProblem::computeJacobian(NonlinearImplicitSystem & sys, const NumericVector<Number> & soln, SparseMatrix<Number> & jacobian)
{
  if (!_has_jacobian || !_const_jacobian)
  {
    _nl.setSolution(soln);

    _nl.zeroVariablesForJacobian();
    _aux.zeroVariablesForJacobian();

    unsigned int n_threads = libMesh::n_threads();

    // Random interface objects
    for (const auto & it : _random_data_objects)
      it.second->updateSeeds(EXEC_NONLINEAR);

    _currently_computing_jacobian = true;

    execTransfers(EXEC_NONLINEAR);
    execMultiApps(EXEC_NONLINEAR);

    for (unsigned int tid = 0; tid < n_threads; tid++)
      reinitScalars(tid);

    computeUserObjects(EXEC_NONLINEAR, Moose::PRE_AUX);

    if (_displaced_problem != NULL)
      _displaced_problem->updateMesh();

    for (unsigned int tid = 0; tid < n_threads; tid++)
    {
      _all_materials.jacobianSetup(tid);
      _functions.jacobianSetup(tid);
    }

    _aux.jacobianSetup();

    _aux.compute(EXEC_NONLINEAR);

    computeUserObjects(EXEC_NONLINEAR, Moose::POST_AUX);

    executeControls(EXEC_NONLINEAR);

    _app.getOutputWarehouse().jacobianSetup();

    _nl.computeJacobian(jacobian);

    _currently_computing_jacobian = false;
    _has_jacobian = true;
  }

  if (_solver_params._type == Moose::ST_JFNK || _solver_params._type == Moose::ST_PJFNK)
  {
    // This call is here to make sure the residual vector is up to date with any decisions that have been made in
    // the Jacobian evaluation.  That is important in JFNK because that residual is used for finite differencing
    computeResidual(sys, soln, *sys.rhs);
    sys.rhs->close();
  }
}

void
FEProblem::computeTransientImplicitJacobian(Real time, const NumericVector<Number> & u, const NumericVector<Number> & udot, Real shift, SparseMatrix<Number> & jacobian)
{
  if (0)
  { // The current interface guarantees that the residual is called before Jacobian, thus udot has already been set
    _nl.setSolutionUDot(udot);
  }
  _nl.duDotDu() = shift;
  NonlinearImplicitSystem &sys = _nl.sys();
  _time = time;
  computeJacobian(sys,u,jacobian);
}


void
FEProblem::computeJacobianBlocks(std::vector<JacobianBlock *> & blocks)
{
  if (_displaced_problem != NULL)
    _displaced_problem->updateMesh();

  _aux.compute(EXEC_NONLINEAR);

  _nl.computeJacobianBlocks(blocks);
}

void
FEProblem::computeJacobianBlock(SparseMatrix<Number> & jacobian, libMesh::System & precond_system, unsigned int ivar, unsigned int jvar)
{
  std::vector<JacobianBlock *> blocks;
  JacobianBlock * block = new JacobianBlock(precond_system, jacobian, ivar, jvar);
  blocks.push_back(block);
  computeJacobianBlocks(blocks);
  delete block;
}

void
FEProblem::computeBounds(NonlinearImplicitSystem & /*sys*/, NumericVector<Number>& lower, NumericVector<Number>& upper)
{
  if (!_nl.hasVector("lower_bound") || !_nl.hasVector("upper_bound")) return;

  NumericVector<Number> & _lower = _nl.getVector("lower_bound");
  NumericVector<Number> & _upper = _nl.getVector("upper_bound");
  _lower.swap(lower);
  _upper.swap(upper);
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
    _all_materials.residualSetup(tid);

  _aux.residualSetup();
  _aux.compute(EXEC_LINEAR);
  _lower.swap(lower);
  _upper.swap(upper);

  checkExceptionAndStopSolve();
}

void
FEProblem::computeNearNullSpace(NonlinearImplicitSystem & /*sys*/, std::vector<NumericVector<Number>*>& sp)
{
  sp.clear();
  for (unsigned int i = 0; i < subspaceDim("NearNullSpace"); ++i) {
    std::stringstream postfix;
    postfix << "_" << i;
    std::string modename = "NearNullSpace" + postfix.str();
    sp.push_back(&_nl.getVector(modename));
  }
}

void
FEProblem::computeNullSpace(NonlinearImplicitSystem & /*sys*/, std::vector<NumericVector<Number>*>& sp)
{
  sp.clear();
  for (unsigned int i = 0; i < subspaceDim("NullSpace"); ++i) {
    std::stringstream postfix;
    postfix << "_" << i;
    sp.push_back(&_nl.getVector("NullSpace"+postfix.str()));
  }
}

void
FEProblem::computeTransposeNullSpace(NonlinearImplicitSystem & /*sys*/, std::vector<NumericVector<Number>*>& sp)
{
  sp.clear();
  for (unsigned int i = 0; i < subspaceDim("TransposeNullSpace"); ++i) {
    std::stringstream postfix;
    postfix << "_" << i;
    sp.push_back(&_nl.getVector("TransposeNullSpace"+postfix.str()));
  }
}

void
FEProblem::computePostCheck(NonlinearImplicitSystem & sys,
                            const NumericVector<Number> & old_soln,
                            NumericVector<Number> & search_direction,
                            NumericVector<Number> & new_soln,
                            bool & changed_search_direction,
                            bool & changed_new_soln)
{
  // This function replaces the old PetscSupport::dampedCheck() function.
  //
  // 1.) Recreate code in PetscSupport::dampedCheck() for constructing
  //     ghosted "soln" and "update" vectors.
  // 2.) Call FEProblem::computeDamping() with these ghost vectors.
  // 3.) Recreate the code in PetscSupport::dampedCheck() to actually update
  //     the solution vector based on the damping, and set the "changed" flags
  //     appropriately.
  Moose::perf_log.push("computePostCheck()", "Execution");

  // MOOSE's FEProblem doesn't update the solution during the
  // postcheck, but FEProblem-derived classes (see e.g.
  // FrictionalContactProblem) might.
  if (_has_dampers || shouldUpdateSolution())
  {
    // We need ghosted versions of new_soln and search_direction (the
    // ones we get from libmesh/PETSc are PARALLEL vectors.  To make
    // our lives simpler, we use the same ghosting pattern as the
    // system's current_local_solution to create new ghosted vectors.

    // Construct zeroed-out clones with the same ghosted dofs as the
    // System's current_local_solution.
    std::unique_ptr<NumericVector<Number> >
      ghosted_solution = sys.current_local_solution->zero_clone(),
      ghosted_search_direction = sys.current_local_solution->zero_clone();

    // Copy values from input vectors into clones with ghosted values.
    *ghosted_solution = new_soln;
    *ghosted_search_direction = search_direction;

    if (_has_dampers)
    {
      // Compute the damping coefficient using the ghosted vectors
      Real damping = computeDamping(*ghosted_solution, *ghosted_search_direction);

      // If some non-trivial damping was computed, update the new_soln
      // vector accordingly.
      if (damping < 1.0)
      {
        new_soln = old_soln;
        new_soln.add(-damping, search_direction);
        changed_new_soln = true;
      }
    }

    if (shouldUpdateSolution())
    {
      // Update the ghosted copy of the new solution, if necessary.
      if (changed_new_soln)
        *ghosted_solution = new_soln;

      bool updated_solution = updateSolution(new_soln, *ghosted_solution);
      if (updated_solution)
        changed_new_soln = true;
    }

  }

  // MOOSE doesn't change the search_direction
  changed_search_direction = false;

  Moose::perf_log.pop("computePostCheck()", "Execution");
}

Real
FEProblem::computeDamping(const NumericVector<Number>& soln, const NumericVector<Number>& update)
{
  Moose::perf_log.push("compute_dampers()", "Execution");

  // Default to no damping
  Real damping = 1.0;

  if (_has_dampers)
  {
    // Save pointer to the current solution
    const NumericVector<Number>* _saved_current_solution = _nl.currentSolution();

    _nl.setSolution(soln);
    // For now, do not re-compute auxiliary variables.  Doing so allows a wild solution increment
    //   to get to the material models, which may not be able to cope with drastically different
    //   values.  Once more complete dependency checking is in place, auxiliary variables (and
    //   material properties) will be computed as needed by dampers.
//    _aux.compute();
    damping = _nl.computeDamping(soln, update);

    // restore saved solution
    _nl.setSolution(*_saved_current_solution);
  }

  Moose::perf_log.pop("compute_dampers()", "Execution");

  return damping;
}

bool
FEProblem::shouldUpdateSolution()
{
  return false;
}

bool
FEProblem::updateSolution(NumericVector<Number>& /*vec_solution*/, NumericVector<Number>& /*ghosted_solution*/)
{
  return false;
}

void
FEProblem::predictorCleanup(NumericVector<Number>& /*ghosted_solution*/)
{
}

void
FEProblem::addDisplacedProblem(MooseSharedPointer<DisplacedProblem> displaced_problem)
{
  _displaced_mesh = &displaced_problem->mesh();
  _displaced_problem = displaced_problem;
}

void
FEProblem::updateGeomSearch(GeometricSearchData::GeometricSearchType type)
{
  _geometric_search_data.update(type);

  if (_displaced_problem)
    _displaced_problem->updateGeomSearch(type);
}

void
FEProblem::possiblyRebuildGeomSearchPatches()
{
  if (_displaced_problem) // Only need to do this if things are moving...
  {
    switch (_mesh.getPatchUpdateStrategy())
    {
      case 0: // Never
        break;
      case 2: // Auto
      {
        Real max = _displaced_problem->geomSearchData().maxPatchPercentage();
        _communicator.max(max);

        // If we haven't moved very far through the patch
        if (max < 0.4)
          break;
      }

      // Let this fall through if things do need to be updated...

      case 1: // Always
        // Flush output here to see the message before the reinitialization, which could take a while
        _console << "\n\nUpdating geometric search patches\n"<<std::endl;

        _geometric_search_data.clearNearestNodeLocators();
        _mesh.updateActiveSemiLocalNodeRange(_ghosted_elems);

        _displaced_problem->geomSearchData().clearNearestNodeLocators();
        _displaced_mesh->updateActiveSemiLocalNodeRange(_ghosted_elems);

        reinitBecauseOfGhostingOrNewGeomObjects();

        // This is needed to reinitialize PETSc output
        initPetscOutput();
    }
  }
}

#ifdef LIBMESH_ENABLE_AMR
void
FEProblem::initialAdaptMesh()
{
  unsigned int n = adaptivity().getInitialSteps();
  _cycles_completed = 0;
  for (unsigned int i = 0; i < n; i++)
  {
    _console << "Initial adaptivity step " << i+1 << " of " << n << std::endl;
    computeIndicators();
    computeMarkers();

    if (_adaptivity.initialAdaptMesh())
    {
      meshChanged();

      //reproject the initial condition
      projectSolution();

      _cycles_completed++;
    }
    else
    {
      _console << "Mesh unchanged, skipping remaining steps..." << std::endl;
      return;
    }
  }
}

void
FEProblem::adaptMesh()
{
  if (!_adaptivity.isAdaptivityDue())
    return;

  unsigned int cycles_per_step = _adaptivity.getCyclesPerStep();
  _cycles_completed = 0;
  for (unsigned int i = 0; i < cycles_per_step; ++i)
  {
    _console << "Adaptivity step " << i+1 << " of " << cycles_per_step << '\n';
    // Markers were already computed once by Executioner
    if (_adaptivity.getRecomputeMarkersFlag() && i > 0)
      computeMarkers();
    if (_adaptivity.adaptMesh())
    {
      meshChanged();
      _cycles_completed++;
    }
    else
    {
      _console << "Mesh unchanged, skipping remaining steps..." << std::endl;
      return;
    }

    // Show adaptivity progress
    _console << std::flush;
  }
}
#endif //LIBMESH_ENABLE_AMR

void
FEProblem::initXFEM(MooseSharedPointer<XFEMInterface> xfem)
{
  _xfem = xfem;
  _xfem->setMesh(&_mesh.getMesh());
  if (_displaced_mesh)
    _xfem->setSecondMesh(&_displaced_mesh->getMesh());
  _xfem->setMaterialData(&_material_data);
  _xfem->setBoundaryMaterialData(&_bnd_material_data);

  unsigned int n_threads = libMesh::n_threads();
  for (unsigned int i = 0; i < n_threads; ++i)
  {
    _assembly[i]->setXFEM(_xfem);
    if (_displaced_problem != NULL)
      _displaced_problem->assembly(i).setXFEM(_xfem);
  }
}

bool
FEProblem::updateMeshXFEM()
{
  bool updated = false;
  if (haveXFEM())
  {
    updated = _xfem->update(_time);
    if (updated)
    {
      meshChanged();
      _xfem->initSolution(_nl, _aux);
      restoreSolutions();
    }
  }
  return updated;
}

void
FEProblem::meshChanged()
{
  if (_material_props.hasStatefulProperties() || _bnd_material_props.hasStatefulProperties())
    _mesh.cacheChangedLists(); // Currently only used with adaptivity and stateful material properties

  // Clear these out because they corresponded to the old mesh
  _ghosted_elems.clear();

  ghostGhostedBoundaries();

  // mesh changed
  _eq.reinit();
  _mesh.meshChanged();

  // Since the Mesh changed, update the PointLocator object used by DiracKernels.
  _dirac_kernel_info.updatePointLocator(_mesh);

  unsigned int n_threads = libMesh::n_threads();

  for (unsigned int i = 0; i < n_threads; ++i)
    _assembly[i]->invalidateCache();

  // Need to redo ghosting
  _geometric_search_data.reinit();

  if (_displaced_problem != NULL)
  {
    _displaced_problem->meshChanged();
    _displaced_mesh->updateActiveSemiLocalNodeRange(_ghosted_elems);
  }

  _mesh.updateActiveSemiLocalNodeRange(_ghosted_elems);

  reinitBecauseOfGhostingOrNewGeomObjects();

  // We need to create new storage for the new elements and copy stateful properties from the old elements.
  if (_has_initialized_stateful && (_material_props.hasStatefulProperties() || _bnd_material_props.hasStatefulProperties()))
  {
    {
      ProjectMaterialProperties pmp(true, *this, _nl, _material_data, _bnd_material_data, _material_props, _bnd_material_props, _assembly);
      Threads::parallel_reduce(*_mesh.refinedElementRange(), pmp);
    }

    {
      ProjectMaterialProperties pmp(false, *this, _nl, _material_data, _bnd_material_data, _material_props, _bnd_material_props, _assembly);
      Threads::parallel_reduce(*_mesh.coarsenedElementRange(), pmp);
    }
  }

  if (_calculate_jacobian_in_uo)
    setVariableAllDoFMap(_uo_jacobian_moose_vars[0]);

  _has_jacobian = false;                    // we have to recompute jacobian when mesh changed

  for (const auto & mci : _notify_when_mesh_changes)
    mci->meshChanged();
}

void
FEProblem::notifyWhenMeshChanges(MeshChangedInterface * mci)
{
  _notify_when_mesh_changes.push_back(mci);
}

void
FEProblem::checkProblemIntegrity()
{
  // Check for unsatisfied actions
  const std::set<SubdomainID> & mesh_subdomains = _mesh.meshSubdomains();

  // Check kernel coverage of subdomains (blocks) in the mesh
  if (_solve && _kernel_coverage_check)
    _nl.checkKernelCoverage(mesh_subdomains);

  // Check materials
  {
#ifdef LIBMESH_ENABLE_AMR
    if (_adaptivity.isOn() && (_material_props.hasStatefulProperties() || _bnd_material_props.hasStatefulProperties()))
    {
      _console << "Using EXPERIMENTAL Stateful Material Property projection with Adaptivity!\n";

      if (n_processors() > 1)
      {
        if (_mesh.uniformRefineLevel() > 0 && _mesh.getMesh().skip_partitioning() == false)
          mooseError("This simulation is using uniform refinement on the mesh, with stateful properties and adaptivity. "
                     "You must skip partitioning to run this case:\nMesh/skip_partitioning=true");

        _console << "\nWarning! Mesh re-partitioning is disabled while using stateful material properties!  This can lead to large load imbalances and degraded performance!!\n\n";
        _mesh.getMesh().skip_partitioning(true);
        if (_displaced_problem)
          _displaced_problem->mesh().getMesh().skip_partitioning(true);
      }
    }
#endif

    std::set<SubdomainID> local_mesh_subs(mesh_subdomains);

    if (_material_coverage_check)
    {
      /**
       * If a material is specified for any block in the simulation, then all blocks must
       * have a material specified.
       */
      bool check_material_coverage = false;
      std::set<SubdomainID> ids = _all_materials.getActiveBlocks();
      for (const auto & id : ids)
      {
        local_mesh_subs.erase(id);
        check_material_coverage = true;
      }

      // also exclude mortar spaces from the material check
      auto & mortar_ifaces = _mesh.getMortarInterfaces();
      for (const auto & mortar_iface : mortar_ifaces)
        local_mesh_subs.erase(mortar_iface->_id);

      // Check Material Coverage
      if (check_material_coverage && !local_mesh_subs.empty())
      {
        std::stringstream extra_subdomain_ids;
        /// unsigned int is necessary to print SubdomainIDs in the statement below
        std::copy (local_mesh_subs.begin(), local_mesh_subs.end(), std::ostream_iterator<unsigned int>(extra_subdomain_ids, " "));

        mooseError("The following blocks from your input mesh do not contain an active material: " + extra_subdomain_ids.str() + "\nWhen ANY mesh block contains a Material object, all blocks must contain a Material object.\n");
      }
    }

    // Check material properties on blocks and boundaries
    checkBlockMatProps();
    //checkBoundaryMatProps();

    // Check that material properties exist when requested by other properties on a given block
    const std::vector<MooseSharedPointer<Material> > & materials = _all_materials.getActiveObjects();
    for (const auto & material : materials)
      material->checkStatefulSanity();

    checkDependMaterialsHelper(_all_materials.getActiveBlockObjects());
  }

  // Check UserObjects and Postprocessors
  checkUserObjects();

  // Verify that we don't have any Element type/Coordinate Type conflicts
  checkCoordinateSystems();

  // If using displacements, verify that the order of the displacement
  // variables matches the order of the elements in the displaced
  // mesh.
  checkDisplacementOrders();
}

void
FEProblem::checkDisplacementOrders()
{
  if (_displaced_problem)
  {
    MeshBase::const_element_iterator
      it = _displaced_mesh->activeLocalElementsBegin(),
      end = _displaced_mesh->activeLocalElementsEnd();

    bool mesh_has_second_order_elements = false;
    for (; it != end; ++it)
    {
      if ((*it)->default_order() == SECOND)
      {
        mesh_has_second_order_elements = true;
        break;
      }
    }

    // We checked our local elements, so take the max over all processors.
    _displaced_mesh->comm().max(mesh_has_second_order_elements);

    // If the Mesh has second order elements, make sure the
    // displacement variables are second-order.
    if (mesh_has_second_order_elements)
    {
      const std::vector<std::string> & displacement_variables =
        _displaced_problem->getDisplacementVarNames();

      for (const auto & var_name : displacement_variables)
      {
        MooseVariable & mv = _displaced_problem->getVariable(/*tid=*/0, var_name);
        if (mv.order() != SECOND)
          mooseError("Error: mesh has SECOND order elements, so all displacement variables must be SECOND order.");
      }
    }
  }
}

void
FEProblem::checkUserObjects()
{
  // Check user_objects block coverage
  std::set<SubdomainID> mesh_subdomains = _mesh.meshSubdomains();
  std::set<SubdomainID> user_objects_blocks;

  // gather names of all user_objects that were defined in the input file
  // and the blocks that they are defined on
  std::set<std::string> names;

  const std::vector<MooseSharedPointer<UserObject> > & objects = _all_user_objects.getActiveObjects();
  for (const auto & obj : objects)
    names.insert(obj->name());

  // See if all referenced blocks are covered
  mesh_subdomains.insert(Moose::ANY_BLOCK_ID);
  std::set<SubdomainID> difference;
  std::set_difference(user_objects_blocks.begin(), user_objects_blocks.end(), mesh_subdomains.begin(), mesh_subdomains.end(),
                      std::inserter(difference, difference.end()));

  if (!difference.empty())
  {
    std::ostringstream oss;
    oss << "One or more UserObjects is referencing a nonexistent block:\n";
    for (const auto & id : difference)
      oss << id << "\n";
    mooseError(oss.str());
  }

  // check that all requested UserObjects were defined in the input file
  for (const auto & it : _pps_data.values())
  {
    if (names.find(it.first) == names.end())
      mooseError("Postprocessor '" + it.first + "' requested but not specified in the input file.");
  }
}


void
FEProblem::checkDependMaterialsHelper(const std::map<SubdomainID, std::vector<MooseSharedPointer<Material> > > & materials_map)
{
  for (const auto & it : materials_map)
  {
    /// These two sets are used to make sure that all dependent props on a block are actually supplied
    std::set<std::string> block_depend_props, block_supplied_props;

    for (const auto & mat1 : it.second)
    {
      const std::set<std::string> & depend_props = mat1->getRequestedItems();
      block_depend_props.insert(depend_props.begin(), depend_props.end());

      // See if any of the active materials supply this property
      for (const auto & mat2 : it.second)
      {
        // Don't check THIS material for a coupled property
        if (mat1 == mat2)
          continue;

        const std::set<std::string> & supplied_props = mat2->Material::getSuppliedItems();
        block_supplied_props.insert(supplied_props.begin(), supplied_props.end());
      }
    }

    // Add zero material properties specific to this block and unrestricted
    block_supplied_props.insert(_zero_block_material_props[it.first].begin(), _zero_block_material_props[it.first].end());
    block_supplied_props.insert(_zero_block_material_props[Moose::ANY_BLOCK_ID].begin(), _zero_block_material_props[Moose::ANY_BLOCK_ID].end());

    // Error check to make sure all properties consumed by materials are supplied on this block
    std::set<std::string> difference;
    std::set_difference(block_depend_props.begin(), block_depend_props.end(), block_supplied_props.begin(), block_supplied_props.end(),
                        std::inserter(difference, difference.end()));

    if (!difference.empty())
    {
      std::ostringstream oss;
      oss << "One or more Material Properties were not supplied on block " << it.first << ":\n";
      for (const auto & name : difference)
        oss << name << "\n";
      mooseError(oss.str());
    }
  }

  // This loop checks that materials are not supplied by multiple Material objects
  for (const auto & it : materials_map)
  {
    const std::vector<MooseSharedPointer<Material> > & materials = it.second;
    std::set<std::string> inner_supplied, outer_supplied;

    for (const auto & outer_mat : materials)
    {
      // Storage for properties for this material (outer) and all other materials (inner)
      outer_supplied = outer_mat->getSuppliedItems();
      inner_supplied.clear();

      // Property to material map for error reporting
      std::map<std::string, std::set<std::string> > prop_to_mat;
      for (const auto & name : outer_supplied)
        prop_to_mat[name].insert(outer_mat->name());

      for (const auto & inner_mat : materials)
      {
        if (outer_mat == inner_mat)
          continue;
        inner_supplied.insert(inner_mat->getSuppliedItems().begin(),
                              inner_mat->getSuppliedItems().end());

        for (const auto & inner_supplied_name : inner_supplied)
          prop_to_mat[inner_supplied_name].insert(inner_mat->name());
      }

      // Test that a property isn't supplied on multiple blocks
      std::set<std::string> intersection;
      std::set_intersection(outer_supplied.begin(), outer_supplied.end(),
                            inner_supplied.begin(), inner_supplied.end(),
                            std::inserter(intersection, intersection.end()));

      if (!intersection.empty())
      {
        std::ostringstream oss;
        oss << "The following material properties are declared on block " << it.first << " by multiple materials:\n";
        oss << ConsoleUtils::indent(2) << std::setw(30) << std::left << "Material Property" << "Material Objects\n";
        for (const auto & outer_name : intersection)
        {
          oss << ConsoleUtils::indent(2) << std::setw(30) << std::left << outer_name;
          for (const auto & inner_name : prop_to_mat[outer_name])
            oss << inner_name << " ";
          oss << '\n';
        }

        oss << "\nThis will result in ambiguous material property calculations and lead to incorrect results.\n";
        mooseWarning(oss.str());
        break;
      }
    }
  }
}


void
FEProblem::checkCoordinateSystems()
{
  MeshBase::const_element_iterator it = _mesh.getMesh().elements_begin();
  MeshBase::const_element_iterator it_end = _mesh.getMesh().elements_end();

  for ( ; it != it_end; ++it)
  {
    SubdomainID sid = (*it)->subdomain_id();
    if (_coord_sys[sid] == Moose::COORD_RZ && (*it)->dim() == 3)
      mooseError("An RZ coordinate system was requested for subdomain " + Moose::stringify(sid) + " which contains 3D elements.");
    if (_coord_sys[sid] == Moose::COORD_RSPHERICAL && (*it)->dim() > 1)
      mooseError("An RSPHERICAL coordinate system was requested for subdomain " + Moose::stringify(sid) + " which contains 2D or 3D elements.");
  }
}

void
FEProblem::serializeSolution()
{
  _nl.serializeSolution();
  _aux.serializeSolution();
}

void
FEProblem::setRestartFile(const std::string & file_name)
{
  _app.setRestart(true);
  _resurrector->setRestartFile(file_name);
}

std::vector<VariableName>
FEProblem::getVariableNames()
{
  std::vector<VariableName> names;

  const std::vector<VariableName> & nl_var_names = _nl.getVariableNames();
  names.insert(names.end(), nl_var_names.begin(), nl_var_names.end());

  const std::vector<VariableName> & aux_var_names = _aux.getVariableNames();
  names.insert(names.end(), aux_var_names.begin(), aux_var_names.end());

  return names;
}

MooseNonlinearConvergenceReason
FEProblem::checkNonlinearConvergence(std::string &msg,
                                     const PetscInt it,
                                     const Real xnorm,
                                     const Real snorm,
                                     const Real fnorm,
                                     const Real rtol,
                                     const Real stol,
                                     const Real abstol,
                                     const PetscInt nfuncs,
                                     const PetscInt max_funcs,
                                     const Real initial_residual_before_preset_bcs,
                                     const Real div_threshold)
{
  NonlinearSystem & system = getNonlinearSystem();
  MooseNonlinearConvergenceReason reason = MOOSE_NONLINEAR_ITERATING;

  // This is the first residual before any iterations have been done,
  // but after PresetBCs (if any) have been imposed on the solution
  // vector.  We save it, and use it to detect convergence if
  // compute_initial_residual_before_preset_bcs=false.
  if (it==0)
    system._initial_residual_after_preset_bcs = fnorm;

  std::ostringstream oss;
  if (fnorm != fnorm)
  {
    oss << "Failed to converge, function norm is NaN\n";
    reason = MOOSE_DIVERGED_FNORM_NAN;
  }
  else if (fnorm < abstol)
  {
    oss << "Converged due to function norm " << fnorm << " < " << abstol << '\n';
    reason = MOOSE_CONVERGED_FNORM_ABS;
  }
  else if (nfuncs >= max_funcs)
  {
    oss << "Exceeded maximum number of function evaluations: " << nfuncs << " > " << max_funcs << '\n';
    reason = MOOSE_DIVERGED_FUNCTION_COUNT;
  }
  else if (it &&
          fnorm > system._last_nl_rnorm &&
          fnorm >= div_threshold)
  {
    oss << "Nonlinear solve was blowing up!\n";
    reason = MOOSE_DIVERGED_LINE_SEARCH;
  }

  if (it && !reason)
  {
    // If compute_initial_residual_before_preset_bcs==false, then use the
    // first residual computed by Petsc to determine convergence.
    Real the_residual = system._compute_initial_residual_before_preset_bcs ? initial_residual_before_preset_bcs : system._initial_residual_after_preset_bcs;
    if (fnorm <= the_residual*rtol)
    {
      oss << "Converged due to function norm " << fnorm << " < " << " (relative tolerance)\n";
      reason = MOOSE_CONVERGED_FNORM_RELATIVE;
    }
    else if (snorm < stol*xnorm)
    {
      oss << "Converged due to small update length: " << snorm << " < " << stol << " * " << xnorm << '\n';
      reason = MOOSE_CONVERGED_SNORM_RELATIVE;
    }
  }

  system._last_nl_rnorm = fnorm;
  system._current_nl_its = static_cast<unsigned int>(it);

  msg = oss.str();
  if (_app.multiAppLevel() > 0)
    MooseUtils::indentMessage(_app.name(), msg);

  return(reason);
}



MooseLinearConvergenceReason
FEProblem::checkLinearConvergence(std::string & /*msg*/,
                                  const PetscInt n,
                                  const Real rnorm,
                                  const Real /*rtol*/,
                                  const Real /*atol*/,
                                  const Real /*dtol*/,
                                  const PetscInt maxits)
{
  if (_fail_next_linear_convergence_check)
  {
    // Unset the flag
    _fail_next_linear_convergence_check = false;
    return MOOSE_DIVERGED_NANORINF;
  }

  // We initialize the reason to something that basically means MOOSE
  // has not made a decision on convergence yet.
  MooseLinearConvergenceReason reason = MOOSE_LINEAR_ITERATING;

  // Get a reference to our Nonlinear System
  NonlinearSystem & system = getNonlinearSystem();

  // If it's the beginning of a new set of iterations, reset
  // last_rnorm, otherwise record the most recent linear residual norm
  // in the NonlinearSystem.
  if (n == 0)
    system._last_rnorm = 1e99;
  else
    system._last_rnorm = rnorm;

  // If the linear residual norm is less than the System's linear absolute
  // step tolerance, we consider it to be converged and set the reason as
  // MOOSE_CONVERGED_RTOL.
  if (std::abs(rnorm - system._last_rnorm) < system._l_abs_step_tol)
    reason = MOOSE_CONVERGED_RTOL;

  // If we hit max its, then we consider that converged (rather than
  // KSP_DIVERGED_ITS).
  if (n >= maxits)
    reason = MOOSE_CONVERGED_ITS;

  // If either of our convergence criteria is met, store the number of linear
  // iterations in the System.
  if (reason == MOOSE_CONVERGED_ITS || reason == MOOSE_CONVERGED_RTOL)
    system._current_l_its.push_back(static_cast<unsigned int>(n));

  return reason;
}


SolverParams &
FEProblem::solverParams()
{
  return _solver_params;
}

void
FEProblem::registerRandomInterface(RandomInterface & random_interface, const std::string & name)
{
  RandomData *random_data;
  if (_random_data_objects.find(name) == _random_data_objects.end())
  {
    random_data = new RandomData(*this, random_interface);
    random_interface.setRandomDataPointer(random_data);

    _random_data_objects[name] = random_data;
  }
  else
    random_interface.setRandomDataPointer(_random_data_objects[name]);
}

bool
FEProblem::needMaterialOnSide(BoundaryID bnd_id, THREAD_ID tid)
{
  if (_bnd_mat_side_cache[tid].find(bnd_id) == _bnd_mat_side_cache[tid].end())
  {
    _bnd_mat_side_cache[tid][bnd_id] = false;

    if (_nl.needMaterialOnSide(bnd_id, tid) || _aux.needMaterialOnSide(bnd_id))
      _bnd_mat_side_cache[tid][bnd_id] = true;

    else if (_side_user_objects.hasActiveBoundaryObjects(bnd_id, tid))
      _bnd_mat_side_cache[tid][bnd_id] = true;
  }

  return _bnd_mat_side_cache[tid][bnd_id];
}

bool
FEProblem::needMaterialOnSide(SubdomainID subdomain_id, THREAD_ID tid)
{
  if (_block_mat_side_cache[tid].find(subdomain_id) == _block_mat_side_cache[tid].end())
  {
    _block_mat_side_cache[tid][subdomain_id] = false;

    if (_nl.needMaterialOnSide(subdomain_id, tid))
      _block_mat_side_cache[tid][subdomain_id] = true;
    else if (_internal_side_user_objects.hasActiveBlockObjects(subdomain_id, tid))
      _block_mat_side_cache[tid][subdomain_id] = true;
  }

  return _block_mat_side_cache[tid][subdomain_id];
>>>>>>> Update variable dependencies prior to running general userobjects
}
