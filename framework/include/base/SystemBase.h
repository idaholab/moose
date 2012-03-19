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

#ifndef SYSTEMBASE_H
#define SYSTEMBASE_H

#include <vector>

#include "Kernel.h"
#include "NodalBC.h"
#include "IntegratedBC.h"
#include "InitialCondition.h"

#include "MooseMesh.h"
#include "VariableWarehouse.h"
#include "Assembly.h"
#include "ParallelUniqueId.h"
#include "SubProblem.h"

// libMesh
#include "equation_systems.h"
#include "dof_map.h"
#include "exodusII_io.h"
#include "nonlinear_implicit_system.h"
#include "quadrature.h"
#include "point.h"

class MooseVariable;

/// Free function used for a libMesh callback
void extraSendList(std::vector<unsigned int> & send_list, void * context);

/// Free function used for a libMesh callback
void extraSparsity(SparsityPattern::Graph & sparsity,
                   std::vector<unsigned int> & n_nz,
                   std::vector<unsigned int> & n_oz,
                   void * context);

/**
 * Base class for a system (of equations)
 */
class SystemBase
{
public:
  SystemBase(SubProblem & subproblem, const std::string & name);
  virtual ~SystemBase() {}

  virtual unsigned int number() = 0;
  virtual MooseMesh & mesh() { return _mesh; }
  virtual SubProblem & subproblem() { return _subproblem; }

  /**
   * Gets the dof map
   */
  virtual DofMap & dofMap() = 0;

  /**
   * Get the reference to the libMesh system
   */
  virtual System & system() = 0;

  /**
   * Initialize the system
   */
  virtual void init() = 0;

  /// Called only once, just before the solve begins so objects can do some precalculations
  virtual void initializeObjects() {};

  /**
   * Update the system (doing libMesh magic)
   */
  virtual void update() = 0;

  /**
   * Solve the system (using libMesh magic)
   */
  virtual void solve() = 0;

  virtual void copyOldSolutions() = 0;
  virtual void restoreSolutions() = 0;

  /**
   * The solution vector that is currently being operated on.
   * This is typically a ghosted vector that comes in from the Nonlinear solver.
   */
  virtual const NumericVector<Number> * & currentSolution() = 0;

  virtual NumericVector<Number> & solution() = 0;
  virtual NumericVector<Number> & solutionOld() = 0;
  virtual NumericVector<Number> & solutionOlder() = 0;

  virtual NumericVector<Number> & solutionUDot() = 0;
  virtual NumericVector<Number> & solutionDuDotDu() = 0;

  /// Get a raw NumericVector
  virtual NumericVector<Number> & getVector(std::string name) = 0;

  /// Returns a reference to a serialized version of the solution vector for this subproblem
  virtual NumericVector<Number> & serializedSolution() = 0;

  virtual NumericVector<Number> & residualCopy() { mooseError("This system does not support getting a copy of the residual"); }
  virtual NumericVector<Number> & residualGhosted() { mooseError("This system does not support getting a ghosted copy of the residual"); }

  /// Will modify the send_list to add all of the extra ghosted dofs for this system
  virtual void augmentSendList(std::vector<unsigned int> & send_list) = 0;

  /// Will modify the sparsity pattern to add logcial geometric connections
  virtual void augmentSparsity(SparsityPattern::Graph & sparsity,
                               std::vector<unsigned int> & n_nz,
                               std::vector<unsigned int> & n_oz) = 0;

  /// Returns true if we are currently computing Jacobian
  virtual bool currentlyComputingJacobian() { return _currently_computing_jacobian; }

  /**
   * Adds a variable to the system
   *
   * @param var_name name of the variable
   * @param type FE type of the variable
   * @param scale_factor the scaling factor for the variable
   * @param active_subdomains a list of subdomain ids this variable is active on
   */
  virtual void addVariable(const std::string & var_name, const FEType & type, Real scale_factor, const std::set< subdomain_id_type > * const active_subdomains = NULL) = 0;

  /**
   * Query a system for a variable
   *
   * @param var_name name of the variable
   * @return true if the variable exists
   */
  virtual bool hasVariable(const std::string & var_name) = 0;
  virtual bool hasScalarVariable(const std::string & var_name) = 0;

  /**
   * Gets a reference to a variable of with specified name
   *
   * @param tid Thread id
   * @param var_name varaible name
   * @return reference the variable (class)
   */
  virtual MooseVariable & getVariable(THREAD_ID tid, const std::string & var_name);

  /**
   * Gets a reference to a variable with specified number
   *
   * @param tid Thread id
   * @param var_number varaible number
   * @return reference the variable (class)
   */
  virtual MooseVariable & getVariable(THREAD_ID tid, unsigned int var_number);

  /**
   * Gets a reference to a scalar variable with specified number
   *
   * @param tid Thread id
   * @param var_number varaible number
   * @return reference the variable (class)
   */
  virtual MooseVariableScalar & getScalarVariable(THREAD_ID tid, const std::string & var_name);

  /**
   * Gets a reference to a variable with specified number
   *
   * @param tid Thread id
   * @param var_number varaible number
   * @return reference the variable (class)
   */
  virtual MooseVariableScalar & getScalarVariable(THREAD_ID tid, unsigned int var_number);

  /**
   * Get the block where a variable of this system is defined
   *
   * @param var_number The number of the variable
   * @return the set of subdomain ids where the variable is active (defined)
   */
  virtual const std::set<subdomain_id_type> * getVariableBlocks(unsigned int var_number);

  /**
   * Get the number of variables in this system
   * @return the number of variables
   */
  virtual unsigned int nVariables() = 0;

  virtual unsigned int nScalarVariables() = 0;

  /// Get minimal quadrature order needed for integrating variables in this system
  virtual Order getMinQuadratureOrder();

  /**
   * Prepare the system for use
   * @param tid ID of the thread
   */
  virtual void prepare(THREAD_ID tid);

  /**
   * Prepare the system for use
   * @param tid ID of the thread
   */
  virtual void prepareNeighbor(THREAD_ID tid);

  /**
   * Reinit an element assembly info
   * @param elem Which element we are reinitializing for
   * @param tid ID of the thread
   */
  virtual void reinitElem(const Elem * elem, THREAD_ID tid);

  /**
   * Reinit assembly info for a side of an element
   * @param elem The element
   * @param side Side of of the element
   * @param bnd_id Boundary id on that side
   * @param tid Thread ID
   */
  virtual void reinitElemFace(const Elem * elem, unsigned int side, unsigned int bnd_id, THREAD_ID tid);

  /**
   * Compute the values of the variables at all the current points.
   */
  virtual void reinitNeighborFace(const Elem * elem, unsigned int side, unsigned int bnd_id, THREAD_ID tid);

  /**
   * Compute the values of the variables at all the current points.
   */
  virtual void reinitNeighbor(const Elem * elem, THREAD_ID tid);

  /**
   * Reinit nodal assembly info
   * @param node Node to reinit for
   * @param tid Thread ID
   */
  virtual void reinitNode(const Node * node, THREAD_ID tid);

  /**
   * Reinit nodal assembly info on a face
   * @param node Node to reinit
   * @param bnd_id Boundary ID
   * @param tid Thread ID
   */
  virtual void reinitNodeFace(const Node * node, unsigned int bnd_id, THREAD_ID tid);

  /**
   * Reinit nodal assembly info for neighbor node
   * @param node Node to reinit for
   * @param tid Thread ID
   */
  virtual void reinitNodeNeighbor(const Node * node, THREAD_ID tid);

  /**
   * Reinit variables at a set of nodes
   * @param nodes List of node ids to reinit
   * @param tid Thread ID
   */
  virtual void reinitNodes(const std::vector<unsigned int> & nodes, THREAD_ID tid);

  /**
   * Reinit scalar varaibles
   * @param tid Thread ID
   */
  virtual void reinitScalars(THREAD_ID tid);

  /**
   * Add info about variable that will be copied
   *
   * @param name Name of the nodal variable being used for copying (name is from the exodusII file)
   * @param timestep Timestep in the file being used
   */
  virtual void addVariableToCopy(const std::string & name, unsigned int timestep) = 0;

protected:
  Problem & _problem;
  SubProblem & _subproblem;
  MooseMesh & _mesh;
  std::string _name;                                                    ///< The name of this system

  bool _currently_computing_jacobian;                                   ///< Whether or not the system is currently computing the Jacobian matrix

  std::vector<VariableWarehouse> _vars;                                 ///< Variable warehouses (one for each thread)
  std::map<unsigned int, std::set<subdomain_id_type> > _var_map;        ///< Map of variables (variable id -> array of subdomains where it lives)
};

/**
 * Information about variables that will be copied
 */
struct VarCopyInfo {
  VarCopyInfo(const std::string & name, unsigned int timestep) :
    _name(name),
    _timestep(timestep)
  {}

  std::string _name;
  unsigned int _timestep;
};

/**
 * Template for wrapping libMesh systems within MOOSE
 */
template<typename T>
class SystemTempl : public SystemBase
{
public:
  SystemTempl(SubProblem & subproblem, const std::string & name, Moose::VarKindType var_kind) :
      SystemBase(subproblem, name),
      _sys(subproblem.es().add_system<T>(_name)),
      _var_kind(var_kind),
      _solution(*_sys.solution),
      _solution_old(*_sys.old_local_solution),
      _solution_older(*_sys.older_local_solution),
      _dummy_sln(NULL)
  {
  }

  virtual ~SystemTempl()
  {
  }

  virtual void addVariable(const std::string & var_name, const FEType & type, Real scale_factor, const std::set< subdomain_id_type > * const active_subdomains = NULL)
  {
    unsigned int var_num = _sys.add_variable(var_name, type, active_subdomains);
    unsigned int mvn = nVariables();                                      // MOOSE variable number
    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
    {
      MooseVariable * var = new MooseVariable(var_num, mvn, type, *this, _subproblem.assembly(tid), _var_kind);
      var->scalingFactor(scale_factor);
      _vars[tid].add(var_name, var);
    }
    if (active_subdomains == NULL)
      _var_map[mvn] = std::set<subdomain_id_type>();
    else
      for (std::set<subdomain_id_type>::iterator it = active_subdomains->begin(); it != active_subdomains->end(); ++it)
        _var_map[mvn].insert(*it);
    _var_names.push_back(var_name);
  }

  /**
   * Adds a scalar variable
   * @param var_name The name of the variable
   * @param order The order of the variable
   */
  virtual void addScalarVariable(const std::string & var_name, Order order, Real scale_factor)
  {
    FEType type(order, SCALAR);
    unsigned int var_num = _sys.add_variable(var_name, type);
    unsigned int msvn = nScalarVariables();                                      // MOOSE scalar variable number
    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
    {
      MooseVariableScalar * var = new MooseVariableScalar(var_num, msvn, *this, _subproblem.assembly(tid), _var_kind);
      var->scalingFactor(scale_factor);
      _vars[tid].add(var_name, var);
    }
  }

  virtual bool hasVariable(const std::string & var_name)
  {
    if (_sys.has_variable(var_name))
      return _sys.variable_type(var_name).family != SCALAR;
    else
      return false;
  }

  virtual bool hasScalarVariable(const std::string & var_name)
  {
    if (_sys.has_variable(var_name))
      return _sys.variable_type(var_name).family == SCALAR;
    else
      return false;
  }

  virtual unsigned int nVariables() { return _vars[0].all().size(); }

  virtual unsigned int nScalarVariables() { return _vars[0].scalars().size(); }

  std::vector<std::string> & getVariableNames() { return _var_names; }

  virtual void computeVariables(const NumericVector<Number>& /*soln*/)
  {
  }

  virtual NumericVector<Number> & solution() { return _solution; }
  virtual NumericVector<Number> & solutionOld() { return _solution_old; }
  virtual NumericVector<Number> & solutionOlder() { return _solution_older; }

  virtual NumericVector<Number> & solutionUDot() { return *_dummy_sln; }
  virtual NumericVector<Number> & solutionDuDotDu() { return *_dummy_sln; }

  virtual NumericVector<Number> & getVector(std::string name) { return _sys.get_vector(name); }

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

  /**
   * Copy current solution into old and older
   */
  virtual void copySolutionsBackwards()
  {
    _sys.update();
    *_sys.older_local_solution = *_sys.current_local_solution;
    *_sys.old_local_solution   = *_sys.current_local_solution;
  }

  /**
   * Shifts the solutions backwards in time
   */
  virtual void copyOldSolutions()
  {
    *_sys.older_local_solution = *_sys.old_local_solution;
    *_sys.old_local_solution = *_sys.current_local_solution;
  }

  /**
   * Restore current solutions (call after your solve failed)
   */
  virtual void restoreSolutions()
  {
    *_sys.current_local_solution = *_sys.old_local_solution;
    *_sys.solution = *_sys.old_local_solution;
    _sys.update();
  }

  virtual void projectSolution (Number fptr(const Point& p,
                                             const Parameters& parameters,
                                             const std::string& sys_name,
                                             const std::string& unknown_name),
                                 Gradient gptr(const Point& p,
                                               const Parameters& parameters,
                                               const std::string& sys_name,
                                               const std::string& unknown_name),
                                 Parameters& parameters) const
  {
    // project the solution only if we have dofs in this system (without this check, libMesh will freak out and crash on us)
    if (_sys.n_dofs() > 0)
      _sys.project_solution(fptr, gptr, parameters);
  }

  /**
   * Get a reference to libMesh system object
   * @return the reference to the libMesh object
   */
  T & sys() { return _sys; }

  /**
   * Gets the number of this system
   * @return The number of this system
   */
  virtual unsigned int number() { return _sys.number(); }
  virtual DofMap & dofMap() { return _sys.get_dof_map(); }
  virtual System & system() { return _sys; }

  virtual void addVariableToCopy(const std::string & name, unsigned int timestep)
  {
    _var_to_copy.push_back(VarCopyInfo(name, timestep));
  }

  void copyVars(ExodusII_IO & io)
  {
    for (std::vector<VarCopyInfo>::iterator it = _var_to_copy.begin();
        it != _var_to_copy.end();
        ++it)
    {
      VarCopyInfo & vci = *it;
      io.copy_nodal_solution(_sys, vci._name, vci._timestep);
    }
    _solution.close();
  }

protected:
  T & _sys;
  Moose::VarKindType _var_kind;                                      ///< default kind of variables in this system
  std::vector<std::string> _var_names;

  NumericVector<Number> & _solution;
  NumericVector<Number> & _solution_old;
  NumericVector<Number> & _solution_older;

  NumericVector<Number> * _dummy_sln;                     // to satisfy the interface

  std::vector<VarCopyInfo> _var_to_copy;
};


#endif /* SYSTEMBASE_H */
