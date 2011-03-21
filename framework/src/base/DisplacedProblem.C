#include "DisplacedProblem.h"
#include "Problem.h"
#include "SubProblem.h"

namespace Moose
{

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

  void operator() (const NodeRange & range) const
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

      if(_problem._nl.sys().has_variable(displacement_name))
      {
         var_nums.push_back(_problem._nl.sys().variable_number(displacement_name));
         var_nums_directions.push_back(i);
      }
      else if(_problem._aux.sys().has_variable(displacement_name))
      {
         aux_var_nums.push_back(_problem._aux.sys().variable_number(displacement_name));
         aux_var_nums_directions.push_back(i);
      }
      else
        mooseError("Undefined variable used for displacements!");
    }

    unsigned int num_var_nums = var_nums.size();
    unsigned int num_aux_var_nums = aux_var_nums.size();


    unsigned int nonlinear_system_number = _problem._nl.sys().number();
    unsigned int aux_system_number = _problem._aux.sys().number();

    NodeRange::const_iterator nd = range.begin();

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
  Mesh & _ref_mesh;
  const NumericVector<Number> & _nl_soln;
  const NumericVector<Number> & _aux_soln;
};


DisplacedProblem::DisplacedProblem(SubProblem & problem, Mesh & displaced_mesh, Mesh & mesh, const std::vector<std::string> & displacements) :
    _problem(problem),
    _mesh(displaced_mesh),
    _eq(displaced_mesh),
    _ref_mesh(mesh),
    _displacements(displacements),
    _nl(_problem, "DisplacedSystem"),
    _aux(_problem, "DisplacedAuxSystem"),
    _nl_solution(NumericVector<Number>::build().release()),
    _aux_solution(NumericVector<Number>::build().release()),
    _geometric_search_data(_problem, _mesh),
    _ex(_eq)
{
  _mesh.prepare();
  _mesh.build_node_list_from_side_list();

  _ex.sequence(true);
}

DisplacedProblem::~DisplacedProblem()
{
  delete _nl_solution;
  delete _aux_solution;
}

void
DisplacedProblem::init()
{
  _eq.init();
  _nl_solution->init(_nl.sys().n_dofs(), false, SERIAL);
  _aux_solution->init(_aux.sys().n_dofs(), false, SERIAL);
}

void
DisplacedProblem::serializeSolution(const NumericVector<Number> & soln, const NumericVector<Number> & aux_soln)
{
  soln.localize(*_nl_solution);
  aux_soln.localize(*_aux_solution);
}

void
DisplacedProblem::updateMesh(const NumericVector<Number> & soln, const NumericVector<Number> & aux_soln)
{
  Moose::perf_log.push("updateDisplacedMesh()","Solve");

  (*_nl.sys().solution) = soln;
  (*_aux.sys().solution) = aux_soln;

  Threads::parallel_for(*_mesh.getActiveNodeRange(), UpdateDisplacedMeshThread(*this));

  Moose::perf_log.pop("updateDisplacedMesh()","Solve");

  // Update the geometric searches that depend on the displaced mesh
  _geometric_search_data.update();
}

Variable &
DisplacedProblem::getVariable(THREAD_ID tid, const std::string & var_name)
{
  if (_nl.hasVariable(var_name))
    return _nl.getVariable(tid, var_name);
  else if (_aux.hasVariable(var_name))
    return _aux.getVariable(tid, var_name);
  else
    mooseError("No variable with name '" + var_name + "'");
}

void
DisplacedProblem::addVariable(const std::string & var_name, const FEType & type, const std::set< subdomain_id_type > * const active_subdomains)
{
  _nl.addVariable(var_name, type, active_subdomains);
//  unsigned int var_num = _nl_sys.add_variable(var_name, type, active_subdomains);
//  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
//  {
//    Moose::Variable * var = new Moose::Variable(tid, var_num, type, *this);
//    _vars[tid].add(var_name, var);
//
////    if (active_subdomains == NULL)
////      _var_map[var].insert(Moose::ANY_BLOCK_ID);
////    else
////      for (std::set<subdomain_id_type>::iterator it = active_subdomains->begin(); it != active_subdomains->end(); ++it)
////        _var_map[var].insert(*it);
////  }
//  }
}

void
DisplacedProblem::addAuxVariable(const std::string & var_name, const FEType & type, const std::set< subdomain_id_type > * const active_subdomains)
{
  _aux.addVariable(var_name, type, active_subdomains);
//  unsigned int var_num = _aux_sys.add_variable(var_name, type, active_subdomains);
//  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
//  {
//    Moose::Variable * var = new Moose::Variable(tid, var_num, type, *this);
//    _vars[tid].add(var_name, var);
//
////    if (active_subdomains == NULL)
////      _var_map[var].insert(Moose::ANY_BLOCK_ID);
////    else
////      for (std::set<subdomain_id_type>::iterator it = active_subdomains->begin(); it != active_subdomains->end(); ++it)
////        _var_map[var].insert(*it);
////  }
//  }
}

void
DisplacedProblem::reinitElem(const Elem * elem, THREAD_ID tid)
{
  std::cerr << "DisplacedProblem::reinitElem(const Elem * elem, THREAD_ID tid)" << std::endl;
}

void
DisplacedProblem::output(Real time)
{
  // FIXME: use proper file_base
  _ex.output("out_displaced", time);
  _ex.meshChanged();
}


} // namespace
