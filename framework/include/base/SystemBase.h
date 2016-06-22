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

#include "VariableWarehouse.h"
#include "ParallelUniqueId.h"
#include "SubProblem.h"
#include "MooseVariableScalar.h"
#include "MooseVariable.h"
#include "DataIO.h"

// libMesh
#include "libmesh/exodusII_io.h"
#include "libmesh/parallel_object.h"
#include "libmesh/dof_map.h"
#include "libmesh/equation_systems.h"
#include "libmesh/numeric_vector.h"

// Forward declarations
class Factory;
class MooseApp;
class MooseVariable;
class MooseMesh;
class SystemBase;

// libMesh forward declarations
namespace libMesh
{
class System;
}

/**
 * ///< Type of coordinate system
 */
void extraSendList(std::vector<dof_id_type> & send_list, void * context);

/**
 * Free function used for a libMesh callback
 */
void extraSparsity(SparsityPattern::Graph & sparsity,
                   std::vector<dof_id_type> & n_nz,
                   std::vector<dof_id_type> & n_oz,
                   void * context);

/**
 * IO Methods for restart, backup and restore.
 */
template<>
void
dataStore(std::ostream & stream, SystemBase & system_base, void * context);

/**
 * IO Methods for restart, backup and restore.
 */
template<>
void
dataLoad(std::istream & stream, SystemBase & system_base, void * context);

/**
 * Base class for a system (of equations)
 *
 */
class SystemBase : public libMesh::ParallelObject
{
public:
  SystemBase(SubProblem & subproblem, const std::string & name);
  virtual ~SystemBase() {}

  virtual unsigned int number() = 0;
  virtual const std::string & name() = 0;
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

  /**
   * Called only once, just before the solve begins so objects can do some precalculations
   */
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
  virtual Number & duDotDu() = 0;

  /**
   * Check if the named vector exists in the system.
   */
  virtual bool hasVector(const std::string & name) = 0;

  /**
   * Get a raw NumericVector
   */
  virtual NumericVector<Number> & getVector(const std::string & name) = 0;

  /**
   * Returns a reference to a serialized version of the solution vector for this subproblem
   */
  virtual NumericVector<Number> & serializedSolution() = 0;

  virtual NumericVector<Number> & residualCopy() { mooseError("This system does not support getting a copy of the residual"); }
  virtual NumericVector<Number> & residualGhosted() { mooseError("This system does not support getting a ghosted copy of the residual"); }

  /**
   * Will modify the send_list to add all of the extra ghosted dofs for this system
   */
  virtual void augmentSendList(std::vector<dof_id_type> & send_list);

  /**
   * Will modify the sparsity pattern to add logical geometric connections
   */
  virtual void augmentSparsity(SparsityPattern::Graph & sparsity,
                               std::vector<dof_id_type> & n_nz,
                               std::vector<dof_id_type> & n_oz) = 0;

  /**
   * Adds a variable to the system
   *
   * @param var_name name of the variable
   * @param type FE type of the variable
   * @param scale_factor the scaling factor for the variable
   * @param active_subdomains a list of subdomain ids this variable is active on
   */
  virtual void addVariable(const std::string & var_name, const FEType & type, Real scale_factor, const std::set< SubdomainID > * const active_subdomains = NULL) = 0;

  /**
   * Query a system for a variable
   *
   * @param var_name name of the variable
   * @return true if the variable exists
   */
  virtual bool hasVariable(const std::string & var_name) = 0;
  virtual bool hasScalarVariable(const std::string & var_name) = 0;

  virtual bool isScalarVariable(unsigned int var_name) = 0;

  /**
   * Gets a reference to a variable of with specified name
   *
   * @param tid Thread id
   * @param var_name variable name
   * @return reference the variable (class)
   */
  virtual MooseVariable & getVariable(THREAD_ID tid, const std::string & var_name);

  /**
   * Gets a reference to a variable with specified number
   *
   * @param tid Thread id
   * @param var_number libMesh variable number
   * @return reference the variable (class)
   */
  virtual MooseVariable & getVariable(THREAD_ID tid, unsigned int var_number);

  /**
   * Gets a reference to a scalar variable with specified number
   *
   * @param tid Thread id
   * @param var_name A string which is the name of the variable to get.
   * @return reference the variable (class)
   */
  virtual MooseVariableScalar & getScalarVariable(THREAD_ID tid, const std::string & var_name);

  /**
   * Gets a reference to a variable with specified number
   *
   * @param tid Thread id
   * @param var_number libMesh variable number
   * @return reference the variable (class)
   */
  virtual MooseVariableScalar & getScalarVariable(THREAD_ID tid, unsigned int var_number);

  /**
   * Get the block where a variable of this system is defined
   *
   * @param var_number The number of the variable
   * @return the set of subdomain ids where the variable is active (defined)
   */
  virtual const std::set<SubdomainID> * getVariableBlocks(unsigned int var_number);

  /**
   * Get the number of variables in this system
   * @return the number of variables
   */
  virtual unsigned int nVariables() = 0;

  /**
   * Adds this variable to the list of variables to be zeroed during each residual evaluation.
   * @param var_name The name of the variable to be zeroed.
   */
  virtual void addVariableToZeroOnResidual(std::string var_name);

  /**
   * Adds this variable to the list of variables to be zeroed during each Jacobian evaluation.
   * @param var_name The name of the variable to be zeroed.
   */
  virtual void addVariableToZeroOnJacobian(std::string var_name);

  /**
   * Zero out the solution for the list of variables passed in.
   *
   * @ param vars_to_be_zeroed The variable names in this vector will have their solutions set to zero after this call
   */
  virtual void zeroVariables(std::vector<std::string> & vars_to_be_zeroed);

  /**
   * Zero out the solution for the variables that were registered as needing to have their solutions
   * zeroed on out on residual evaluation by a call to addVariableToZeroOnResidual()
   */
  virtual void zeroVariablesForResidual();

  /**
   * Zero out the solution for the variables that were registered as needing to have their solutions
   * zeroed on out on Jacobian evaluation by a call to addVariableToZeroOnResidual()
   */
  virtual void zeroVariablesForJacobian();

  /**
   * Get minimal quadrature order needed for integrating variables in this system
   * @return The minimal order of quadrature
   */
  virtual Order getMinQuadratureOrder();

  /**
   * Prepare the system for use
   * @param tid ID of the thread
   */
  virtual void prepare(THREAD_ID tid);

  /**
   * Prepare the system for use on sides
   *
   * This will try to reuse the preparation done on the element.
   *
   * @param tid ID of the thread
   * @param resize_data Pass True if this system needs to resize residual and jacobian datastructures based on preparing this face
   */
  virtual void prepareFace(THREAD_ID tid, bool resize_data);

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
  virtual void reinitElemFace(const Elem * elem, unsigned int side, BoundaryID bnd_id, THREAD_ID tid);

  /**
   * Compute the values of the variables at all the current points.
   */
  virtual void reinitNeighborFace(const Elem * elem, unsigned int side, BoundaryID bnd_id, THREAD_ID tid);

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
  virtual void reinitNodeFace(const Node * node, BoundaryID bnd_id, THREAD_ID tid);

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
  virtual void reinitNodes(const std::vector<dof_id_type> & nodes, THREAD_ID tid);

  /**
   * Reinit variables at a set of neighbor nodes
   * @param nodes List of node ids to reinit
   * @param tid Thread ID
   */
  virtual void reinitNodesNeighbor(const std::vector<dof_id_type> & nodes, THREAD_ID tid);

  /**
   * Reinit scalar varaibles
   * @param tid Thread ID
   */
  virtual void reinitScalars(THREAD_ID tid);

  /**
   * Add info about variable that will be copied
   *
   * @param dest_name Name of the nodal variable being used for copying into (name is from the exodusII file)
   * @param source_name Name of the nodal variable being used for copying from (name is from the exodusII file)
   * @param timestep Timestep in the file being used
   */
  virtual void addVariableToCopy(const std::string & dest_name, const std::string & source_name, const std::string & timestep) = 0;

  const std::vector<MooseVariable *> & getVariables(THREAD_ID tid) { return _vars[tid].variables(); }
  const std::vector<MooseVariableScalar *> & getScalarVariables(THREAD_ID tid) { return _vars[tid].scalars(); }

  const std::set<SubdomainID> & getSubdomainsForVar(unsigned int var_number) const { return _var_map.at(var_number); }

protected:
  SubProblem & _subproblem;

  MooseApp & _app;
  Factory & _factory;

  MooseMesh & _mesh;
  /// The name of this system
  std::string _name;

  /// Variable warehouses (one for each thread)
  std::vector<VariableWarehouse> _vars;
  /// Map of variables (variable id -> array of subdomains where it lives)
  std::map<unsigned int, std::set<SubdomainID> > _var_map;

  std::vector<std::string> _vars_to_be_zeroed_on_residual;
  std::vector<std::string> _vars_to_be_zeroed_on_jacobian;
};

/**
 * Information about variables that will be copied
 */
struct VarCopyInfo {
  VarCopyInfo(const std::string & dest_name, const std::string & source_name, const std::string & timestep) :
    _dest_name(dest_name),
    _source_name(source_name),
    _timestep(timestep)
  {}

  std::string _dest_name;
  std::string _source_name;
  std::string _timestep;
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
      _dummy_vec(NULL),
      _saved_old(NULL),
      _saved_older(NULL)
  {
  }

  virtual ~SystemTempl()
  {
  }

  virtual const std::string & name()
  {
    return _sys.name();
  }

  virtual void addVariable(const std::string & var_name, const FEType & type, Real scale_factor, const std::set< SubdomainID > * const active_subdomains = NULL)
  {
    unsigned int var_num = _sys.add_variable(var_name, type, active_subdomains);
    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
    {
      //FIXME: we cannot refer fetype in libMesh at this point, so we will just make a copy in MooseVariableBase.
      MooseVariable * var = new MooseVariable(var_num, type, *this, _subproblem.assembly(tid), _var_kind);
      var->scalingFactor(scale_factor);
      _vars[tid].add(var_name, var);
    }
    if (active_subdomains == NULL)
      _var_map[var_num] = std::set<SubdomainID>();
    else
      for (std::set<SubdomainID>::iterator it = active_subdomains->begin(); it != active_subdomains->end(); ++it)
        _var_map[var_num].insert(*it);
  }

  /**
   * Adds a scalar variable
   * @param var_name The name of the variable
   * @param order The order of the variable
   * @param scale_factor The scaling factor to be used with this scalar variable
   */
  virtual void addScalarVariable(const std::string & var_name, Order order, Real scale_factor, const std::set< SubdomainID > * const active_subdomains = NULL)
  {
    FEType type(order, SCALAR);
    unsigned int var_num = _sys.add_variable(var_name, type, active_subdomains);
    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
    {
      //FIXME: we cannot refer fetype in libMesh at this point, so we will just make a copy in MooseVariableBase.
      MooseVariableScalar * var = new MooseVariableScalar(var_num, type, *this, _subproblem.assembly(tid), _var_kind);
      var->scalingFactor(scale_factor);
      _vars[tid].add(var_name, var);
    }
    if (active_subdomains == NULL)
      _var_map[var_num] = std::set<SubdomainID>();
    else
      for (std::set<SubdomainID>::iterator it = active_subdomains->begin(); it != active_subdomains->end(); ++it)
        _var_map[var_num].insert(*it);
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

  virtual bool isScalarVariable(unsigned int var_num)
  {
    return (_sys.variable(var_num).type().family == SCALAR);
  }

  virtual unsigned int nVariables() { return _vars[0].names().size(); }

  const std::vector<VariableName> & getVariableNames() const { return _vars[0].names(); }

  virtual void computeVariables(const NumericVector<Number> & /*soln*/)
  {
  }

  virtual NumericVector<Number> & solution() { return _solution; }
  virtual NumericVector<Number> & solutionOld() { return _solution_old; }
  virtual NumericVector<Number> & solutionOlder() { return _solution_older; }

  virtual NumericVector<Number> & solutionUDot() { return *_dummy_vec; }
  virtual Number & duDotDu() { return _du_dot_du; }

  /**
   * Check if the named vector exists in the system.
   */
  virtual bool hasVector(const std::string & name) { return _sys.have_vector(name); }

  /**
   * Get a raw NumericVector with the given name.
   */
  virtual NumericVector<Number> & getVector(const std::string & name) { return _sys.get_vector(name); }

  /**
   * Adds a solution length vector to the system.
   *
   * @param vector_name The name of the vector.
   * @param project Whether or not to project this vector when doing mesh refinement.
   *                If the vector is just going to be recomputed then there is no need to project it.
   * @param type What type of parallel vector.  This is usually either PARALLEL or GHOSTED.
   *                                            GHOSTED is needed if you are going to be accessing off-processor entries.
   *                                            The ghosting pattern is the same as the solution vector.
   */
  virtual NumericVector<Number> & addVector(const std::string & vector_name, const bool project, const ParallelType type)
  {
    if (hasVector(vector_name))
      return getVector(vector_name);

    NumericVector<Number> * vec = &_sys.add_vector(vector_name, project, type);
    return *vec;
  }

  /**
   * Save the old and older solutions.
   */
  virtual void saveOldSolutions()
  {
    if (!_saved_old)
      _saved_old = &addVector("save_solution_old", false, PARALLEL);
    if (!_saved_older)
      _saved_older = &addVector("save_solution_older", false, PARALLEL);
    *_saved_old = solutionOld();
    *_saved_older = solutionOlder();
  }

  /**
   * Restore the old and older solutions when the saved solutions present.
   */
  virtual void restoreOldSolutions()
  {
    if (_saved_old)
    {
      solutionOld() = *_saved_old;
      removeVector("save_solution_old");
      _saved_old = NULL;
    }
    if (_saved_older)
    {
      solutionOlder() = *_saved_older;
      removeVector("save_solution_older");
      _saved_older = NULL;
    }
  }

  /**
   * Remove a vector from the system with the given name.
   */
  virtual void removeVector(const std::string & name) { _sys.remove_vector(name); }

  virtual NumericVector<Number> & residualVector(Moose::KernelType /*type*/) { return *_dummy_vec; }

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

  virtual void addVariableToCopy(const std::string & dest_name, const std::string & source_name, const std::string & timestep)
  {
    _var_to_copy.push_back(VarCopyInfo(dest_name, source_name, timestep));
  }

  void copyVars(ExodusII_IO & io)
  {
    int n_steps = io.get_num_time_steps();

    bool did_copy = false;
    for (std::vector<VarCopyInfo>::iterator it = _var_to_copy.begin();
        it != _var_to_copy.end();
        ++it)
    {
      VarCopyInfo & vci = *it;
      int timestep = -1;

      if (vci._timestep == "LATEST")
        // Use the last time step in the file from which to retrieve the solution
        timestep = n_steps;
      else
      {
        std::istringstream ss(vci._timestep);
        if (!(ss >> timestep) || timestep > n_steps)
          mooseError("Invalid value passed as \"initial_from_file_timestep\". Expected \"LATEST\" or a valid integer between 1 and "
                     << n_steps << " inclusive, received " << vci._timestep);
      }

      did_copy = true;
      if (getVariable(0, vci._dest_name).isNodal())
        io.copy_nodal_solution(_sys, vci._dest_name, vci._source_name, timestep);
      else
        io.copy_elemental_solution(_sys, vci._dest_name, vci._source_name, timestep);
    }

    if (did_copy)
      _solution.close();
  }

protected:
  T & _sys;
  /// default kind of variables in this system
  Moose::VarKindType _var_kind;

  NumericVector<Number> & _solution;
  NumericVector<Number> & _solution_old;
  NumericVector<Number> & _solution_older;
  Real _du_dot_du;

  NumericVector<Number> * _dummy_vec;                     // to satisfy the interface

  std::vector<VarCopyInfo> _var_to_copy;

  // Used for saving old solutions so that they wont be accidentally changed
  NumericVector<Real> * _saved_old;
  NumericVector<Real> * _saved_older;
};


#define PARALLEL_TRY

#define PARALLEL_CATCH _fe_problem.checkExceptionAndStopSolve();


#endif /* SYSTEMBASE_H */
