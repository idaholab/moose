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

#include "AuxiliarySystem.h"
#include "MProblem.h"
#include "Factory.h"
#include "AuxKernel.h"
#include "MaterialData.h"
#include "AssemblyData.h"
#include "GeometricSearchData.h"
#include "ComputeNodalAuxVarsThread.h"
#include "ComputeNodalAuxBcsThread.h"
#include "ComputeElemAuxVarsThread.h"

#include "quadrature_gauss.h"
#include "node_range.h"

// AuxiliarySystem ////////

AuxiliarySystem::AuxiliarySystem(MProblem & subproblem, const std::string & name) :
    SystemTempl<TransientExplicitSystem>(subproblem, name),
    _mproblem(subproblem),
    _serialized_solution(*NumericVector<Number>::build().release()),
    _need_serialized_solution(false)
{
  _sys.attach_init_function(Moose::initial_condition);

  _nodal_vars.resize(libMesh::n_threads());
  _elem_vars.resize(libMesh::n_threads());

  _data.resize(libMesh::n_threads());
}

AuxiliarySystem::~AuxiliarySystem()
{
  delete &_serialized_solution;
}

void
AuxiliarySystem::init()
{
  dofMap().attach_extra_send_list_function(&extraSendList, this);


  if(_need_serialized_solution)
    _serialized_solution.init(_sys.n_dofs(), false, SERIAL);
}

void
AuxiliarySystem::initialSetup()
{
  for(unsigned int i=0; i<libMesh::n_threads(); i++)
  {
    _auxs(EXEC_RESIDUAL)[i].initialSetup();
    _auxs(EXEC_TIMESTEP)[i].initialSetup();
  }
}

void
AuxiliarySystem::timestepSetup()
{
  for(unsigned int i=0; i<libMesh::n_threads(); i++)
  {
    _auxs(EXEC_RESIDUAL)[i].timestepSetup();
    _auxs(EXEC_TIMESTEP)[i].timestepSetup();
  }
}

void
AuxiliarySystem::residualSetup()
{
  for(unsigned int i=0; i<libMesh::n_threads(); i++)
    _auxs(EXEC_RESIDUAL)[i].residualSetup();
}

void
AuxiliarySystem::jacobianSetup()
{
  for(unsigned int i=0; i<libMesh::n_threads(); i++)
    _auxs(EXEC_RESIDUAL)[i].jacobianSetup();
}

void
AuxiliarySystem::addVariable(const std::string & var_name, const FEType & type, Real scale_factor, const std::set< subdomain_id_type > * const active_subdomains/* = NULL*/)
{
  unsigned int var_num = _sys.add_variable(var_name, type, active_subdomains);
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    MooseVariable * var = new MooseVariable(var_num, type, *this, _subproblem.assembly(tid));
    var->scalingFactor(scale_factor);
    _vars[tid].add(var_name, var);
    if (var->feType().family == LAGRANGE)
      _nodal_vars[tid][var_name] = var;
    else
      _elem_vars[tid][var_name] = var;

    if (active_subdomains == NULL)
      _var_map[var_num] = std::set<subdomain_id_type>();
    else
      for (std::set<subdomain_id_type>::iterator it = active_subdomains->begin(); it != active_subdomains->end(); ++it)
        _var_map[var_num].insert(*it);
  }
}

void
AuxiliarySystem::addKernel(const  std::string & kernel_name, const std::string & name, InputParameters parameters)
{
  parameters.set<AuxiliarySystem *>("_aux_sys") = this;
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    parameters.set<THREAD_ID>("_tid") = tid;
    parameters.set<MaterialData *>("_material_data") = _mproblem._material_data[tid];

    AuxKernel *kernel = static_cast<AuxKernel *>(Factory::instance()->create(kernel_name, name, parameters));
    mooseAssert(kernel != NULL, "Not an AuxKernel object");

    std::set<subdomain_id_type> blk_ids;
    if (!parameters.isParamValid("block"))
      blk_ids = _var_map[kernel->variable().number()];
    else
    {
      std::vector<unsigned int> blocks = parameters.get<std::vector<unsigned int> >("block");
      for (unsigned int i=0; i<blocks.size(); ++i)
      {
        if (_var_map[kernel->variable().number()].count(blocks[i]) > 0 || _var_map[kernel->variable().number()].size() == 0)
          blk_ids.insert(blocks[i]);
        else
          mooseError("AuxKernel (" + kernel->name() + "): block outside of the domain of the variable");
      }
    }

    _auxs(kernel->execFlag())[tid].addAuxKernel(kernel, blk_ids);
  }
}

void
AuxiliarySystem::addBoundaryCondition(const std::string & bc_name, const std::string & name, InputParameters parameters)
{
  parameters.set<AuxiliarySystem *>("_aux_sys") = this;
  std::vector<unsigned int> boundaries = parameters.get<std::vector<unsigned int> >("boundary");

  for (unsigned int i=0; i<boundaries.size(); ++i)
  {
    parameters.set<unsigned int>("_boundary_id") = boundaries[i];
    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
    {
      parameters.set<THREAD_ID>("_tid") = tid;
      parameters.set<MaterialData *>("_material_data") = _mproblem._bnd_material_data[tid];

      std::vector<unsigned int> boundaries = parameters.get<std::vector<unsigned int> >("boundary");

      AuxKernel * bc = static_cast<AuxKernel *>(Factory::instance()->create(bc_name, name, parameters));
      mooseAssert(bc != NULL, "Not a AuxBoundaryCondition object");

      _auxs(bc->execFlag())[tid].addActiveBC(boundaries[i], bc);

      _vars[tid].addBoundaryVar(boundaries[i], &bc->variable());
    }
  }
}

void
AuxiliarySystem::reinitElem(const Elem * /*elem*/, THREAD_ID tid)
{
  for (std::map<std::string, MooseVariable *>::iterator it = _nodal_vars[tid].begin(); it != _nodal_vars[tid].end(); ++it)
  {
    MooseVariable *var = it->second;
    var->reinit();
    var->computeElemValues();
  }

  for (std::map<std::string, MooseVariable *>::iterator it = _elem_vars[tid].begin(); it != _elem_vars[tid].end(); ++it)
  {
    MooseVariable *var = it->second;
    var->reinit_aux();
    var->computeElemValues();
  }
}

NumericVector<Number> &
AuxiliarySystem::serializedSolution()
{
  _need_serialized_solution = true;
  return _serialized_solution;
}

void
AuxiliarySystem::serializeSolution()
{
  if(_need_serialized_solution && _sys.n_dofs() > 0)            // libMesh does not like serializing of empty vectors
    solution().localize(_serialized_solution);
}

void
AuxiliarySystem::augmentSendList(std::vector<unsigned int> & send_list)
{
  std::set<unsigned int> & ghosted_elems = _mproblem._ghosted_elems;

  DofMap & dof_map = dofMap();

  std::vector<unsigned int> dof_indices;

  for(std::set<unsigned int>::iterator elem_id = ghosted_elems.begin();
      elem_id != ghosted_elems.end();
      ++elem_id)
  {
    dof_map.dof_indices(_mesh.elem(*elem_id), dof_indices);

    for(unsigned int i=0; i<dof_indices.size(); i++)
      // Only need to ghost it if it's actually not on this processor
      if(dof_indices[i] < dof_map.first_dof() || dof_indices[i] >= dof_map.end_dof())
        send_list.push_back(dof_indices[i]);
  }  
}

void
AuxiliarySystem::compute(ExecFlagType type/* = EXEC_RESIDUAL*/)
{
  //If there aren't any auxiliary variables just return
  if(_sys.n_vars() == 0)
    return;

  computeNodalVars(_auxs(type));
  _sys.update();
  computeElementalVars(_auxs(type));
  solution().close();
  _sys.update();

  if(_need_serialized_solution)
    serializeSolution();
}

void
AuxiliarySystem::computeNodalVars(std::vector<AuxWarehouse> & auxs)
{
  SubdomainIterator subdomain_begin = _mesh.meshSubdomains().begin();
  SubdomainIterator subdomain_end = _mesh.meshSubdomains().end();
  SubdomainIterator subdomain_it;

  // Do we have some kernels to evaluate?
  AuxKernelIterator aux_begin = auxs[0].activeNodalAuxKernelsBegin();
  AuxKernelIterator aux_end = auxs[0].activeNodalAuxKernelsEnd();
  bool have_block_kernels = false;
  for(subdomain_it = subdomain_begin; subdomain_it != subdomain_end; ++subdomain_it)
  {
    AuxKernelIterator block_nodal_aux_begin = auxs[0].activeBlockNodalAuxKernelsBegin(*subdomain_it);
    AuxKernelIterator block_nodal_aux_end = auxs[0].activeBlockNodalAuxKernelsEnd(*subdomain_it);
    have_block_kernels |= (block_nodal_aux_begin != block_nodal_aux_end);
  }

  Moose::perf_log.push("update_aux_vars_nodal()","Solve");
  if (aux_begin != aux_end || have_block_kernels)
  {
    ConstNodeRange & range = *_mesh.getLocalNodeRange();
    ComputeNodalAuxVarsThread navt(_problem, *this, auxs);
    Threads::parallel_reduce(range, navt);
  }
  Moose::perf_log.pop("update_aux_vars_nodal()","Solve");

  //Boundary AuxKernels
  Moose::perf_log.push("update_aux_vars_nodal_bcs()","Solve");
  // after converting this into NodeRange, we can run it in parallel
  ConstBndNodeRange & bnd_nodes = *_mesh.getBoundaryNodeRange();
  ComputeNodalAuxBcsThread nabt(_problem, *this, auxs);
  Threads::parallel_reduce(bnd_nodes, nabt);
  Moose::perf_log.pop("update_aux_vars_nodal_bcs()","Solve");
}

void
AuxiliarySystem::computeElementalVars(std::vector<AuxWarehouse> & auxs)
{
  Moose::perf_log.push("update_aux_vars_elemental()","Solve");
  ConstElemRange & range = *_mesh.getActiveLocalElementRange();
  ComputeElemAuxVarsThread eavt(_mproblem, *this, auxs);
  Threads::parallel_reduce(range, eavt);
  Moose::perf_log.pop("update_aux_vars_elemental()","Solve");
}
