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

#include "DisplacedProblem.h"
#include "Problem.h"
#include "SubProblem.h"

class UpdateDisplacedMeshThread
{
public:
  UpdateDisplacedMeshThread(DisplacedProblem & problem) :
      _problem(problem),
      _ref_mesh(_problem.refMesh()),
      _nl_soln(*_problem._nl_solution),
      _aux_soln(*_problem._aux_solution)
  {
  }

  void operator() (const SemiLocalNodeRange & range) const
  {
    ParallelUniqueId puid;

    std::vector<std::string> & displacement_variables = _problem._displacements;
    unsigned int num_displacements = displacement_variables.size();

    std::vector<unsigned int> var_nums;
    std::vector<unsigned int> var_nums_directions;

    std::vector<unsigned int> aux_var_nums;
    std::vector<unsigned int> aux_var_nums_directions;

    for(unsigned int i=0; i<num_displacements; i++)
    {
      std::string displacement_name = displacement_variables[i];

      if(_problem._displaced_nl.sys().has_variable(displacement_name))
      {
         var_nums.push_back(_problem._displaced_nl.sys().variable_number(displacement_name));
         var_nums_directions.push_back(i);
      }
      else if(_problem._displaced_aux.sys().has_variable(displacement_name))
      {
         aux_var_nums.push_back(_problem._displaced_aux.sys().variable_number(displacement_name));
         aux_var_nums_directions.push_back(i);
      }
      else
        mooseError("Undefined variable used for displacements!");
    }

    unsigned int num_var_nums = var_nums.size();
    unsigned int num_aux_var_nums = aux_var_nums.size();

    unsigned int nonlinear_system_number = _problem._displaced_nl.sys().number();
    unsigned int aux_system_number = _problem._displaced_aux.sys().number();

    SemiLocalNodeRange::const_iterator nd = range.begin();

    for (nd = range.begin() ; nd != range.end(); ++nd)
    {
      Node & displaced_node = *(*nd);

      Node & reference_node = _ref_mesh.node(displaced_node.id());

      for(unsigned int i=0; i<num_var_nums; i++)
      {
        unsigned int direction = var_nums_directions[i];
        displaced_node(direction) = reference_node(direction) + _nl_soln(reference_node.dof_number(nonlinear_system_number, var_nums[i], 0));
      }

      for(unsigned int i=0; i<num_aux_var_nums; i++)
      {
        unsigned int direction = aux_var_nums_directions[i];
        displaced_node(direction) = reference_node(direction) + _aux_soln(reference_node.dof_number(aux_system_number, aux_var_nums[i], 0));
      }
    }
  }

protected:
  DisplacedProblem & _problem;
  MooseMesh & _ref_mesh;
  const NumericVector<Number> & _nl_soln;
  const NumericVector<Number> & _aux_soln;
};


DisplacedProblem::DisplacedProblem(MProblem & mproblem, MooseMesh & displaced_mesh, const std::vector<std::string> & displacements) :
    SubProblemInterface(),
    _problem(*mproblem.parent()),
    _mproblem(mproblem),
    _mesh(displaced_mesh),
    _eq(displaced_mesh),
    _ref_mesh(_mproblem.mesh()),
    _displacements(displacements),
    _displaced_nl(*this, _mproblem.getNonlinearSystem(), "DisplacedSystem"),
    _displaced_aux(*this, _mproblem.getAuxiliarySystem(), "DisplacedAuxSystem"),
    _geometric_search_data(_mproblem, _mesh),
    _ex(_eq)
{
  _ex.sequence(true);

  unsigned int n_threads = libMesh::n_threads();
  _asm_info.resize(n_threads);
  _asm_block.resize(n_threads);
  for (unsigned int i = 0; i < n_threads; ++i)
  {
    _asm_info[i] = new AssemblyData(_mesh);
    _asm_block[i] = new AsmBlock(_displaced_nl, _mproblem.getNonlinearSystem().couplingMatrix(), i);
  }
}

DisplacedProblem::~DisplacedProblem()
{
  for (unsigned int i = 0; i < libMesh::n_threads(); ++i)
  {
    delete _asm_info[i];
    delete _asm_block[i];
  }
}

void
DisplacedProblem::createQRules(QuadratureType type, Order order)
{
  for (unsigned int tid = 0; tid < libMesh::n_threads(); ++tid)
    _asm_info[tid]->createQRules(type, order);
}

void
DisplacedProblem::init()
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
    _asm_block[tid]->init();

  _displaced_nl.init();
  _displaced_aux.init();

  Moose::setup_perf_log.push("DisplacedProblem::init::eq.init()","Setup");
  _eq.init();
  Moose::setup_perf_log.pop("DisplacedProblem::init::eq.init()","Setup");

  _eq.print_info(std::cout);

  Moose::setup_perf_log.push("DisplacedProblem::init::meshChanged()","Setup");
  _mesh.meshChanged();
  Moose::setup_perf_log.pop("DisplacedProblem::init::meshChanged()","Setup");

}

void
DisplacedProblem::updateMesh(const NumericVector<Number> & soln, const NumericVector<Number> & aux_soln)
{
  Moose::perf_log.push("updateDisplacedMesh()","Solve");

  (*_displaced_nl.sys().solution) = soln;
  (*_displaced_aux.sys().solution) = aux_soln;

  _nl_solution = &soln;
  _aux_solution = &aux_soln;

  Threads::parallel_for(*_mesh.getActiveSemiLocalNodeRange(), UpdateDisplacedMeshThread(*this));

  // Update the geometric searches that depend on the displaced mesh
  _geometric_search_data.update();

  Moose::perf_log.pop("updateDisplacedMesh()","Solve");
}

bool
DisplacedProblem::hasVariable(const std::string & var_name)
{
  if (_displaced_nl.hasVariable(var_name))
    return true;
  else if (_displaced_aux.hasVariable(var_name))
    return true;
  else
    return false;
}

MooseVariable &
DisplacedProblem::getVariable(THREAD_ID tid, const std::string & var_name)
{
  if (_displaced_nl.hasVariable(var_name))
    return _displaced_nl.getVariable(tid, var_name);
  else if (_displaced_aux.hasVariable(var_name))
    return _displaced_aux.getVariable(tid, var_name);
  else
    mooseError("No variable with name '" + var_name + "'");
}

void
DisplacedProblem::addVariable(const std::string & var_name, const FEType & type, Real scale_factor, const std::set< subdomain_id_type > * const active_subdomains)
{
  _displaced_nl.addVariable(var_name, type, scale_factor, active_subdomains);
}

void
DisplacedProblem::addAuxVariable(const std::string & var_name, const FEType & type, const std::set< subdomain_id_type > * const active_subdomains)
{
  _displaced_aux.addVariable(var_name, type, 1.0, active_subdomains);
}

void
DisplacedProblem::prepare(const Elem * elem, THREAD_ID tid)
{
  _asm_info[tid]->reinit(elem);

  _displaced_nl.prepare(tid);
  _displaced_aux.prepare(tid);
  _asm_block[tid]->prepare();
}

void
DisplacedProblem::prepare(const Elem * elem, unsigned int ivar, unsigned int jvar, const std::vector<unsigned int> & dof_indices, THREAD_ID tid)
{
  _asm_info[tid]->reinit(elem);

  _displaced_nl.prepare(tid);
  _displaced_aux.prepare(tid);
  _asm_block[tid]->prepareBlock(ivar, jvar, dof_indices);
}

bool
DisplacedProblem::reinitDirac(const Elem * elem, THREAD_ID tid)
{
  std::set<Point> & points_set = _dirac_kernel_info._points[elem];

  bool have_points = points_set.size();

  if(have_points)
  {
    std::vector<Point> points(points_set.size());
    std::copy(points_set.begin(), points_set.end(), points.begin());

    _asm_info[tid]->reinitAtPhysical(elem, points);

    _displaced_nl.prepare(tid);
    _displaced_aux.prepare(tid);
    _asm_block[tid]->prepare();

    reinitElem(elem, tid);
  }

  return have_points;
}


void
DisplacedProblem::reinitElem(const Elem * elem, THREAD_ID tid)
{
  _displaced_nl.reinitElem(elem, tid);
  _displaced_aux.reinitElem(elem, tid);
}

void
DisplacedProblem::reinitElemFace(const Elem * elem, unsigned int side, unsigned int bnd_id, THREAD_ID tid)
{
  _asm_info[tid]->reinit(elem, side);
  _displaced_nl.reinitElemFace(elem, side, bnd_id, tid);
  _displaced_aux.reinitElemFace(elem, side, bnd_id, tid);
}

void
DisplacedProblem::reinitNode(const Node * node, THREAD_ID tid)
{
  _asm_info[tid]->reinit(node);
  _displaced_nl.reinitNode(node, tid);
  _displaced_aux.reinitNode(node, tid);
}

void
DisplacedProblem::reinitNodeFace(const Node * node, unsigned int bnd_id, THREAD_ID tid)
{
  _asm_info[tid]->reinit(node);
  _displaced_nl.reinitNodeFace(node, bnd_id, tid);
  _displaced_aux.reinitNodeFace(node, bnd_id, tid);
}

void
DisplacedProblem::getDiracElements(std::set<const Elem *> & elems)
{
  elems =_dirac_kernel_info._elements;
}

void
DisplacedProblem::clearDiracInfo()
{
  _dirac_kernel_info.clearPoints();
}

AsmBlock &
DisplacedProblem::asmBlock(THREAD_ID tid)
{
  return *_asm_block[tid];
}

void
DisplacedProblem::addResidual(NumericVector<Number> & residual, THREAD_ID tid)
{
  _asm_block[tid]->addResidual(residual);
}

void
DisplacedProblem::addResidualNeighbor(NumericVector<Number> & residual, THREAD_ID tid)
{
  _asm_block[tid]->addResidualNeighbor(residual);
}

void
DisplacedProblem::addJacobian(SparseMatrix<Number> & jacobian, THREAD_ID tid)
{
  _asm_block[tid]->addJacobian(jacobian);
}

void
DisplacedProblem::addJacobianNeighbor(SparseMatrix<Number> & jacobian, THREAD_ID tid)
{
  _asm_block[tid]->addJacobianNeighbor(jacobian);
}

void
DisplacedProblem::addJacobianBlock(SparseMatrix<Number> & jacobian, unsigned int ivar, unsigned int jvar, const DofMap & dof_map, std::vector<unsigned int> & dof_indices, THREAD_ID tid)
{
  _asm_block[tid]->addJacobianBlock(jacobian, ivar, jvar, dof_map, dof_indices);
}

void
DisplacedProblem::addJacobianNeighbor(SparseMatrix<Number> & jacobian, unsigned int ivar, unsigned int jvar, const DofMap & dof_map, std::vector<unsigned int> & dof_indices, std::vector<unsigned int> & neighbor_dof_indices, THREAD_ID tid)
{
  _asm_block[tid]->addJacobianNeighbor(jacobian, ivar, jvar, dof_map, dof_indices, neighbor_dof_indices);
}

void
DisplacedProblem::prepareShapes(unsigned int var, THREAD_ID tid)
{
  _asm_block[tid]->copyShapes(var);
}

void
DisplacedProblem::prepareFaceShapes(unsigned int var, THREAD_ID tid)
{
  _asm_block[tid]->copyFaceShapes(var);
}

void
DisplacedProblem::prepareNeighborShapes(unsigned int var, THREAD_ID tid)
{
  _asm_block[tid]->copyNeighborShapes(var);
}

void
DisplacedProblem::updateGeomSearch()
{
  _geometric_search_data.update();
}

void
DisplacedProblem::output()
{
  // FIXME: use proper file_base
  _ex.output("out_displaced", _mproblem.time());
  _ex.meshChanged();
}

void
DisplacedProblem::meshChanged()
{
  // mesh changed
  _eq.reinit();
  _mesh.meshChanged();
  _geometric_search_data.update();
}

