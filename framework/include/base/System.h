#ifndef SYSTEM_H_
#define SYSTEM_H_

#include "Kernel.h"
#include "NodalBC.h"
#include "IntegratedBC.h"
#include "InitialCondition.h"

#include "ProblemInterface.h"
#include "Mesh.h"
#include "VariableWarehouse.h"

// libMesh
#include "equation_systems.h"
#include "dof_map.h"
#include "exodusII_io.h"


namespace Moose {

class Variable;

class System
{
public:
  System(ProblemInterface & problem, const std::string & name);

  virtual unsigned int number() = 0;
  virtual ProblemInterface & problem() { return _problem; }
  virtual DofMap & dofMap() = 0;

  virtual void init() = 0;
  virtual void update() = 0;
  virtual void solve() = 0;

  virtual void copyOldSolutions() = 0;
  virtual void restoreSolutions() = 0;

  virtual NumericVector<Number> & solution() = 0;
  virtual NumericVector<Number> & solutionOld() = 0;
  virtual NumericVector<Number> & solutionOlder() = 0;
  virtual NumericVector<Number> & solutionUDot() = 0;
  virtual NumericVector<Number> & solutionDuDotDu() = 0;

  virtual void addVariable(const std::string & var_name, const FEType & type, const std::set< subdomain_id_type > * const active_subdomains = NULL) = 0;
  virtual bool hasVariable(const std::string & var_name) = 0;

  virtual Variable & getVariable(THREAD_ID tid, const std::string & var_name);
  virtual int nVariables() = 0;

  /// Get minimal quadrature order needed for integrating variables in this system
  virtual Order getMinQuadratureOrder();
  virtual void prepare(THREAD_ID tid);
  virtual void reinitElem(const Elem * elem, THREAD_ID tid);
  virtual void reinitElemFace(const Elem * elem, unsigned int side, unsigned int bnd_id, THREAD_ID tid);
  virtual void reinitNode(const Node * node, THREAD_ID tid);
  virtual void reinitNodeFace(const Node * node, unsigned int bnd_id, THREAD_ID tid);

  virtual void copyNodalValues(ExodusII_IO & io, const std::string & nodal_var_name, unsigned int timestep) = 0;

  VariableWarehouse & vars(THREAD_ID tid) { return _vars[tid]; }

protected:
  ProblemInterface & _problem;
  Mesh & _mesh;
  std::string _name;

  // TODO: move this to problem (that will keep track of which variable is where), System will only keep a list of variables that created and will free them in d-tor
  std::vector<VariableWarehouse> _vars;
  std::map<unsigned int, std::set<unsigned int> > _var_map;             /// The list of blocks for a given variable number
};


template<typename T>
class SystemTempl : public System
{
public:
  SystemTempl(ProblemInterface & problem, const std::string & name) :
    System(problem, name),
    _sys(problem.es().add_system<T>(_name)),
    _solution(*_sys.solution),
    _solution_old(*_sys.old_local_solution),
    _solution_older(*_sys.older_local_solution),
    _solution_u_dot(_sys.add_vector("u_dot", false, GHOSTED)),
    _solution_du_dot_du(_sys.add_vector("du_dot_du", false, GHOSTED)),
    _residual_old(_sys.add_vector("residual_old", false, GHOSTED))
  {
  }

  virtual ~SystemTempl()
  {
  }

  virtual void addVariable(const std::string & var_name, const FEType & type, const std::set< subdomain_id_type > * const active_subdomains = NULL)
  {
    unsigned int var_num = _sys.add_variable(var_name, type, active_subdomains);
    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
    {
      Moose::Variable * var = new Moose::Variable(var_num, type, *this, _problem.assembly(tid));
      _vars[tid].add(var_name, var);

      if (active_subdomains == NULL)
        _var_map[var_num].insert(Moose::ANY_BLOCK_ID);
      else
        for (std::set<subdomain_id_type>::iterator it = active_subdomains->begin(); it != active_subdomains->end(); ++it)
          _var_map[var_num].insert(*it);
    }
  }

  virtual bool hasVariable(const std::string & var_name)
  {
    return _sys.has_variable(var_name);
  }

  virtual int nVariables() { return _sys.n_vars(); }

  virtual void computeVariables(const NumericVector<Number>& /*soln*/)
  {
  }

  virtual NumericVector<Number> & solution() { return _solution; }
  virtual NumericVector<Number> & solutionOld() { return _solution_old; }
  virtual NumericVector<Number> & solutionOlder() { return _solution_older; }

  virtual NumericVector<Number> & solutionUDot() { return _solution_u_dot; }
  virtual NumericVector<Number> & solutionDuDotDu() { return _solution_du_dot_du; }

  virtual void init()
  {
  }

  virtual void update()
  {
    _sys.update();
  }

  virtual void solve()
  {
    _sys.solve();
  }

  virtual void copySolutionsBackwards()
  {
    *_sys.older_local_solution = *_sys.current_local_solution;
    *_sys.old_local_solution   = *_sys.current_local_solution;
  }

  virtual void copyOldSolutions()
  {
    *_sys.older_local_solution = *_sys.old_local_solution;
    *_sys.old_local_solution = *_sys.current_local_solution;
  }

  virtual void restoreSolutions()
  {
    *_sys.current_local_solution = *_sys.old_local_solution;
    *_sys.solution = *_sys.old_local_solution;
  }

  virtual void copyNodalValues(ExodusII_IO & io, const std::string & nodal_var_name, unsigned int timestep)
  {
    io.copy_nodal_solution(_sys, nodal_var_name, timestep);
  }

  T & sys() { return _sys; }
  virtual unsigned int number() { return _sys.number(); }
  virtual DofMap & dofMap() { return _sys.get_dof_map(); }

protected:
  T & _sys;

  NumericVector<Number> & _solution;
  NumericVector<Number> & _solution_old;
  NumericVector<Number> & _solution_older;

  NumericVector<Number> & _solution_u_dot;
  NumericVector<Number> & _solution_du_dot_du;
  NumericVector<Number> & _residual_old;                /// residual evaluated at the old time step
};

} // namespace

#endif /* SYSTEM_H_ */
