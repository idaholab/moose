#include "AuxiliarySystem.h"
#include "SubProblem.h"
#include "ImplicitSystem.h"
#include "Factory.h"
#include "AuxKernel.h"

#include "quadrature_gauss.h"


namespace Moose {

// threads /////////////////

  class ComputeNodalAuxThread
  {
  protected:
    AuxiliarySystem & _sys;
    SubProblem & _problem;

  public:
    ComputeNodalAuxThread(AuxiliarySystem & sys) :
      _sys(sys),
      _problem(_sys.problem())
    {
    }

    void operator() (const ConstNodeRange & range) const
    {
      ParallelUniqueId puid;
      THREAD_ID tid = puid.id;

      for (ConstNodeRange::const_iterator node_it = range.begin() ; node_it != range.end(); ++node_it)
      {
        const Node * node = *node_it;

        _problem.reinitNode(node, tid);
        for (std::vector<AuxKernel *>::iterator it = _sys._nodal_kernels[tid].begin(); it != _sys._nodal_kernels[tid].end(); ++it)
          (*it)->compute(_sys.solution());
      }
    }
  };

  class ComputeElemAuxThread
  {
  protected:
    AuxiliarySystem & _sys;
    SubProblem & _problem;

  public:
    ComputeElemAuxThread(AuxiliarySystem & sys) :
      _sys(sys),
      _problem(sys.problem())
    {
    }

    void operator() (const ConstElemRange & range) const
    {
      ParallelUniqueId puid;
      THREAD_ID tid = puid.id;

      const unsigned int dim = _sys._mesh.dimension();

      for (ConstElemRange::const_iterator elem_it = range.begin() ; elem_it != range.end(); ++elem_it)
      {
        const Elem * elem = *elem_it;

//        unsigned int cur_subdomain = elem->subdomain_id();

//      block_element_aux_it =_auxs[0].activeBlockElementAuxKernelsBegin(cur_subdomain);
//      block_element_aux_end = _auxs[0].activeBlockElementAuxKernelsEnd(cur_subdomain);
//
//      if(block_element_aux_it != block_element_aux_end || aux_begin != aux_end)
//      {
//        reinitAuxKernels(0, soln, *elem);

          QGauss qrule (dim, FIFTH);
          _problem.attachQuadratureRule(&qrule, tid);
          _problem.reinitElem(elem, tid);

          //Compute the area of the element
          _sys._data[tid]._current_volume = 0;
          //Just use any old JxW... they are all actually the same
          Variable * var = *(_sys._vars[tid].all().begin());
          const std::vector<Real> & jxw = var->JxW();

//        if (_problem.geomType() == Moose::XYZ)
//        {
            for (unsigned int qp = 0; qp < var->qRule()->n_points(); qp++)
              _sys._data[tid]._current_volume += jxw[qp];
//        }
//        else if (_problem.geomType() == Moose::CYLINDRICAL)
//        {
//          const std::vector<Point> & q_point = *(_element_data._q_point.begin()->second);
//          for (unsigned int qp = 0; qp < _element_data._qrule->n_points(); qp++)
//            _current_volume += q_point[qp](0) * jxw[qp];
//        }
//        else
//          mooseError("geom_type must either be XYZ or CYLINDRICAL\n");


//        if(cur_subdomain != subdomain)
//        {
//          subdomain = cur_subdomain;
//
//          _element_data[0]->_material = _materials[0].getMaterials(subdomain);
//          for(aux_it=aux_begin;aux_it!=aux_end;aux_it++)
//            (*aux_it)->subdomainSetup();
//        }
//      }

//      for(; block_element_aux_it != block_element_aux_end; ++block_element_aux_it)
//        (*block_element_aux_it)->computeAndStore();

          for (std::vector<AuxKernel *>::iterator it = _sys._elem_kernels[tid].begin(); it != _sys._elem_kernels[tid].end(); ++it)
            (*it)->compute(_sys.solution());

#if 0
        // Now do the block nodal aux kernels
        block_nodal_aux_it =_auxs[0].activeBlockNodalAuxKernelsBegin(cur_subdomain);
        block_nodal_aux_end = _auxs[0].activeBlockNodalAuxKernelsEnd(cur_subdomain);

        if(block_nodal_aux_it != block_nodal_aux_end)
        {
          for(unsigned int nd = 0; nd < elem->n_nodes(); ++nd)
          {
            Node & node = *elem->get_node(nd);

            if(node.processor_id() == libMesh::processor_id())
            {
              reinitAuxKernels(0, soln, node);

              for(block_nodal_aux_it =_auxs[0].activeBlockNodalAuxKernelsBegin(cur_subdomain);
                  block_nodal_aux_it != block_nodal_aux_end;
                  ++block_nodal_aux_it)
                (*block_nodal_aux_it)->computeAndStore();
            }
          }
        }
#endif
      }
    }
  };

// AuxiliarySystem ////////

AuxiliarySystem::AuxiliarySystem(SubProblem & problem, const std::string & name) :
    SystemTempl<TransientExplicitSystem>(problem, name)
{
  _sys.attach_init_function(Moose::initial_condition);
//  _sys.add_vector("temp");

  _vars.resize(libMesh::n_threads());
  _nodal_vars.resize(libMesh::n_threads());
  _elem_vars.resize(libMesh::n_threads());

  _nodal_kernels.resize(libMesh::n_threads());
  _elem_kernels.resize(libMesh::n_threads());

  _data.resize(libMesh::n_threads());
}

void
AuxiliarySystem::addVariable(const std::string & var_name, const FEType & type, const std::set< subdomain_id_type > * const active_subdomains/* = NULL*/)
{
  unsigned int var_num = _sys.add_variable(var_name, type, active_subdomains);
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    Variable * var = new Variable(var_num, _mesh.dimension(), type, *this);
    _vars[tid].add(var_name, var);
    if (var->feType().family == LAGRANGE)
      _nodal_vars[tid][var_name] = var;
    else
      _elem_vars[tid][var_name] = var;
  }
}

void
AuxiliarySystem::addKernel(const  std::string & kernel_name, const std::string & name, InputParameters parameters)
{
  parameters.set<AuxiliarySystem *>("_aux_sys") = this;
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    parameters.set<THREAD_ID>("_tid") = tid;

    AuxKernel *kernel = static_cast<AuxKernel *>(Factory::instance()->create(kernel_name, name, parameters));
    mooseAssert(kernel != NULL, "Not an AuxKernel object");

//    std::set<unsigned int> blk_ids;
//    if (!parameters.isParamValid("block"))
//      blk_ids = _aux_var_map[aux->variable()];
//    else
//    {
//      std::vector<unsigned int> blocks = parameters.get<std::vector<unsigned int> >("block");
//      for (unsigned int i=0; i<blocks.size(); ++i)
//      {
//        if (_aux_var_map[aux->variable()].count(blocks[i]) > 0 || _aux_var_map[aux->variable()].count(Moose::ANY_BLOCK_ID) > 0)
//          blk_ids.insert(blocks[i]);
//        else
//          mooseError("AuxKernel (" + aux->name() + "): block outside of the domain of the variable");
//      }
//    }

    if (kernel->isNodal())
      _nodal_kernels[tid].push_back(kernel);
    else
      _elem_kernels[tid].push_back(kernel);
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
    // no threading since we do not do nodal BCs in threads (yet)
//    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
    {
//      parameters.set<THREAD_ID>("_tid") = tid;
      parameters.set<THREAD_ID>("_tid") = 0;
      AuxKernel * bc = static_cast<AuxKernel *>(Factory::instance()->create(bc_name, name, parameters));
      mooseAssert(bc != NULL, "Not a AuxBoundaryCondition object");

      _nodal_bcs[boundaries[i]].push_back(bc);
//      _boundary_vars[0][boundaries[i]].insert(&bc->variable());
      _vars[0].addBoundaryVar(boundaries[i], &bc->variable());
    }
  }
}

void
AuxiliarySystem::reinitElem(const Elem * elem, THREAD_ID tid)
{
  for (std::map<std::string, Variable *>::iterator it = _nodal_vars[tid].begin(); it != _nodal_vars[tid].end(); ++it)
  {
    Variable *var = it->second;
    var->reinit(elem);
    var->sizeResidual();
    var->sizeJacobianBlock();
    var->computeElemValues();
  }

  for (std::map<std::string, Variable *>::iterator it = _elem_vars[tid].begin(); it != _elem_vars[tid].end(); ++it)
  {
    Variable *var = it->second;
    var->reinit_aux(elem);
    var->sizeResidual();
    var->sizeJacobianBlock();
    var->computeElemValues();
  }
}

void
AuxiliarySystem::compute()
{
  if (_nodal_vars[0].size() > 0)
  {
#if 0
    AuxKernelIterator aux_begin = _auxs[0].activeNodalAuxKernelsBegin();
    AuxKernelIterator aux_end = _auxs[0].activeNodalAuxKernelsEnd();
    AuxKernelIterator aux_it = aux_begin;
    for(aux_it = aux_begin; aux_it != aux_end; ++aux_it)
      (*aux_it)->setup();
#endif

    ConstNodeRange & node_range = *_mesh.getLocalNodeRange();
    ComputeNodalAuxThread cn(*this);
    Threads::parallel_for(node_range, cn);

    // boundary aux kernels
#if 0
    const std::set<short int> & boundary_ids = _mesh.get_boundary_ids();
    for (std::set<short int>::const_iterator it = boundary_ids.begin(); it != boundary_ids.end(); ++it)
    {
      short int id = *it;

      aux_begin = _auxs[0].activeAuxBCsBegin(id);
      aux_end = _auxs[0].activeAuxBCsEnd(id);

      for(aux_it = aux_begin; aux_it != aux_end; ++aux_it)
        (*aux_it)->setup();
    }
#endif

    std::vector<unsigned int> nodes;
    std::vector<short int> ids;

    _mesh.build_node_list(nodes, ids);

    const unsigned int n_nodes = nodes.size();
    for (unsigned int i = 0; i < n_nodes; i++)
    {
      unsigned int boundary_id = ids[i];
      Node * node = &_mesh.node(nodes[i]);

      // nonlinear variables
      _problem.reinitNodeFace(node, boundary_id, 0);

      for (std::vector<AuxKernel *>::iterator it = _nodal_bcs[boundary_id].begin(); it != _nodal_bcs[boundary_id].end(); ++it)
        (*it)->compute(solution());
    }
  }

  // Update the element aux vars
  if (_elem_vars[0].size() > 0)
  {
#if 0
    aux_begin = _auxs[0].activeElementAuxKernelsBegin();
    aux_end = _auxs[0].activeElementAuxKernelsEnd();
    aux_it = aux_begin;
    for(aux_it = aux_begin; aux_it != aux_end; ++aux_it)
      (*aux_it)->setup();
    aux_it = aux_begin;
#endif

    ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();
    ComputeElemAuxThread ce(*this);
    Threads::parallel_for(elem_range, ce);

  }

  solution().close();
  (*_sys.solution) = _solution;
}

} // namespace
