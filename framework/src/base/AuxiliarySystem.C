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

#include "quadrature_gauss.h"


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

  _auxs.resize(libMesh::n_threads());
  _auxs_ts.resize(libMesh::n_threads());
  _data.resize(libMesh::n_threads());
}

AuxiliarySystem::~AuxiliarySystem()
{
  delete &_serialized_solution;
}

void
AuxiliarySystem::init()
{
  _serialized_solution.init(_sys.n_dofs(), false, SERIAL);
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

    if (kernel->ts())
      _auxs_ts[tid].addAuxKernel(kernel, blk_ids);
    else
      _auxs[tid].addAuxKernel(kernel, blk_ids);
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

      if (bc->ts())
      {
        _auxs_ts[tid].addActiveBC(boundaries[i], bc);
        _auxs_ts[tid].addBC(bc);
      }
      else
      {
        _auxs[tid].addActiveBC(boundaries[i], bc);
        _auxs[tid].addBC(bc);
      }
      _vars[tid].addBoundaryVar(boundaries[i], &bc->variable());
    }
  }
}

void
AuxiliarySystem::reinitElem(const Elem * elem, THREAD_ID tid)
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
  if(_need_serialized_solution)
    solution().localize(_serialized_solution);
}

void
AuxiliarySystem::compute()
{
  computeInternal(_auxs);
  serializeSolution();
}

void
AuxiliarySystem::compute_ts()
{
  computeInternal(_auxs_ts);
}

inline
void
AuxiliarySystem::setupKernels(AuxKernelIterator & begin, AuxKernelIterator & end)
{
  for(AuxKernelIterator it = begin; it != end; ++it)
    (*it)->setup();
}

void
AuxiliarySystem::computeNodalVars(std::vector<AuxWarehouse> & auxs)
{
  AuxKernelIterator aux_begin = auxs[0].activeNodalAuxKernelsBegin();
  AuxKernelIterator aux_end = auxs[0].activeNodalAuxKernelsEnd();

  SubdomainIterator subdomain_begin = _mesh.meshSubdomains().begin();
  SubdomainIterator subdomain_end = _mesh.meshSubdomains().end();
  SubdomainIterator subdomain_it;

  bool have_block_kernels = false;

  // Call setup() on global kernels
  setupKernels(aux_begin, aux_end);
  // Call setup on all of the Nodal block AuxKernels
  for(subdomain_it = subdomain_begin; subdomain_it != subdomain_end; ++subdomain_it)
  {
    AuxKernelIterator block_nodal_aux_begin = auxs[0].activeBlockNodalAuxKernelsBegin(*subdomain_it);
    AuxKernelIterator block_nodal_aux_end = auxs[0].activeBlockNodalAuxKernelsEnd(*subdomain_it);
    setupKernels(block_nodal_aux_begin, block_nodal_aux_end);
    have_block_kernels |= (block_nodal_aux_begin != block_nodal_aux_end);
  }

  if (aux_begin != aux_end || have_block_kernels)
  {
    ConstNodeRange & node_range = *_mesh.getLocalNodeRange();
    for (ConstNodeRange::const_iterator node_it = node_range.begin() ; node_it != node_range.end(); ++node_it)
    {
      const Node * node = *node_it;

//      if(unlikely(_calculate_element_time))
//        startNodeTiming(node->id());

      _problem.reinitNode(node, 0);

      // compute global aux kernels
      for(AuxKernelIterator aux_it = aux_begin; aux_it != aux_end; ++aux_it)
        (*aux_it)->compute();

      const std::vector<subdomain_id_type> & block_ids = _mesh.getNodeBlockIds(*node);
      for (std::vector<subdomain_id_type>::const_iterator block_it = block_ids.begin(); block_it != block_ids.end(); ++block_it)
      {
        AuxKernelIterator block_nodal_aux_begin = auxs[0].activeBlockNodalAuxKernelsBegin(*block_it);
        AuxKernelIterator block_nodal_aux_end = auxs[0].activeBlockNodalAuxKernelsEnd(*block_it);

        for(AuxKernelIterator aux_it = block_nodal_aux_begin; aux_it != block_nodal_aux_end; ++aux_it)
          (*aux_it)->compute();
      }

//      if(unlikely(_calculate_element_time))
//        stopNodeTiming(node->id());
    }
  }

  //Boundary AuxKernels

  const std::set<short int> & boundary_ids = _mesh.get_boundary_ids();

  for(std::set<short int>::const_iterator it = boundary_ids.begin(); it != boundary_ids.end(); ++it)
  {
    short int id = *it;

    aux_begin = _auxs[0].activeAuxBCsBegin(id);
    aux_end = _auxs[0].activeAuxBCsEnd(id);

    setupKernels(aux_begin, aux_end);
  }

  const std::vector<unsigned int> & nodes = _mesh.getBoundaryNodeListNodes();
  const std::vector<short int> & ids = _mesh.getBoundaryNodeListIds();
  const unsigned int n_nodes = nodes.size();

  for(unsigned int i=0; i<n_nodes; i++)
  {
    aux_begin = auxs[0].activeAuxBCsBegin(ids[i]);
    aux_end = auxs[0].activeAuxBCsEnd(ids[i]);

    if(aux_begin != aux_end)
    {
      Node & node = _mesh.node(nodes[i]);

//      if(unlikely(_calculate_element_time))
//        startNodeTiming(node.id());

      if(node.processor_id() == libMesh::processor_id())
      {
        _problem.reinitNodeFace(&node, ids[i], 0);

        for(AuxKernelIterator aux_it=aux_begin; aux_it != aux_end; ++aux_it)
          (*aux_it)->compute();
      }

//      if(unlikely(_calculate_element_time))
//        stopNodeTiming(node.id());
    }
  }
}

void
AuxiliarySystem::computeElementalVars(std::vector<AuxWarehouse> & auxs)
{
  // Update the element aux vars
  AuxKernelIterator aux_begin = auxs[0].activeElementAuxKernelsBegin();
  AuxKernelIterator aux_end = auxs[0].activeElementAuxKernelsEnd();
  setupKernels(aux_begin, aux_end);

  SubdomainIterator subdomain_begin = _mesh.meshSubdomains().begin();
  SubdomainIterator subdomain_end = _mesh.meshSubdomains().end();
  SubdomainIterator subdomain_it;

  // Call setup on all of the Elemental block AuxKernels
  for(subdomain_it = subdomain_begin; subdomain_it != subdomain_end; ++subdomain_it)
  {
    AuxKernelIterator block_element_aux_begin = auxs[0].activeBlockElementAuxKernelsBegin(*subdomain_it);
    AuxKernelIterator block_element_aux_end = auxs[0].activeBlockElementAuxKernelsEnd(*subdomain_it);
    setupKernels(block_element_aux_begin, block_element_aux_end);
  }

  unsigned int subdomain = std::numeric_limits<unsigned int>::max();
  ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();
  for (ConstElemRange::const_iterator elem_it = elem_range.begin() ; elem_it != elem_range.end(); ++elem_it)
  {
    const Elem * elem = *elem_it;

    unsigned int cur_subdomain = elem->subdomain_id();

//    if(unlikely(_calculate_element_time))
//      startElementTiming(elem->id());

    AuxKernelIterator block_element_aux_it = auxs[0].activeBlockElementAuxKernelsBegin(cur_subdomain);
    AuxKernelIterator block_element_aux_end = auxs[0].activeBlockElementAuxKernelsEnd(cur_subdomain);

    if(block_element_aux_it != block_element_aux_end || aux_begin != aux_end)
    {
      _problem.prepare(elem, 0);
      _problem.reinitElem(elem, 0);
      _problem.reinitMaterials(elem->subdomain_id(), 0);

      //Compute the area of the element
      _data[0]._current_volume = _subproblem.assembly(0).computeVolume();

      if(cur_subdomain != subdomain)
      {
        subdomain = cur_subdomain;

        for(AuxKernelIterator aux_it=aux_begin;aux_it!=aux_end;aux_it++)
          (*aux_it)->subdomainSetup();
      }

      // block
      for(; block_element_aux_it != block_element_aux_end; ++block_element_aux_it)
        (*block_element_aux_it)->compute();

      // global
      for(AuxKernelIterator aux_it=aux_begin;aux_it!=aux_end;aux_it++)
        (*aux_it)->compute();
    }

//    if(unlikely(_calculate_element_time))
//      stopElementTiming(elem->id());
  }
}

void
AuxiliarySystem::computeInternal(std::vector<AuxWarehouse> & auxs)
{
  //If there aren't any auxiliary variables just return
  if(_sys.n_vars() == 0)
    return;

  Moose::perf_log.push("update_aux_vars()","Solve");

  computeNodalVars(auxs);
  _sys.update();
  computeElementalVars(auxs);
  solution().close();
  _sys.update();

  Moose::perf_log.pop("update_aux_vars()","Solve");
}
