#ifndef SUBPROBLEM_H_
#define SUBPROBLEM_H_

#include "Mesh.h"
#include "Variable.h"
#include "InitialCondition.h"
#include "MaterialProperty.h"
#include "Function.h"
#include "ParallelUniqueId.h"
#include "Problem.h"

// libMesh include
#include "equation_systems.h"
#include "transient_system.h"
#include "nonlinear_implicit_system.h"
#include "numeric_vector.h"
#include "sparse_matrix.h"

namespace Moose {

class ImplicitSystem;

/**
 * Generic class for solving nonlinear problems
 *
 */
class SubProblem : public Problem
{
public:
  SubProblem(Mesh &mesh, Problem * parent = NULL);
  virtual ~SubProblem();

  virtual Problem * parent() { return _parent; }

  virtual ImplicitSystem & getNonlinearSystem() = 0;

  /**
   * Get reference to all-purpose parameters
   */
  Parameters & parameters() { return _pars; }

  EquationSystems & es() { return _eq; }
  Mesh & mesh() { return _mesh; }

  virtual bool hasVariable(const std::string & var_name);
  virtual Variable & getVariable(THREAD_ID tid, const std::string & var_name);

  // Solve /////
  virtual void init();

  virtual void update();
  virtual void solve();
  virtual bool converged() = 0;

  // Time stepping /////

  /**
   *
   */
  virtual void onTimestepBegin() = 0;
  virtual void onTimestepEnd() = 0;

  virtual void copySolutionsBackwards();

  virtual Real & time() { return _time; }
  virtual int & timeStep() { return _t_step; }
  virtual Real & dt() { return _dt; }
  virtual Real & dtOld() { return _dt_old; }

  void transient(bool trans) { _transient = trans; }
  bool transient() { return _transient; }

  // ICs /////
  void addInitialCondition(const std::string & ic_name, const std::string & name, InputParameters parameters, std::string var_name);
  void addInitialCondition(const std::string & var_name, Real value); 

  // Functions /////
  void addFunction(std::string type, const std::string & name, InputParameters parameters);
  Function & getFunction(const std::string & name, THREAD_ID tid = 0);


  Number initialValue (const Point & p, const Parameters & parameters, const std::string & /*sys_name*/, const std::string & var_name);
  Gradient initialGradient (const Point & p, const Parameters & /*parameters*/, const std::string & /*sys_name*/, const std::string & var_name);

  void initialCondition(EquationSystems & es, const std::string & system_name);

  // Materials /////

  MaterialProperties & materialProps() { return _material_props; }
  MaterialProperties & materialPropsOld() { return _material_props_old; }
  MaterialProperties & materialPropsOlder() { return _material_props_older; }

  virtual void updateMaterials();

  virtual void dump();

protected:
  Problem * _parent;
  Mesh & _mesh;
  EquationSystems _eq;


  bool _transient;
  Real & _time;
  int & _t_step;
  Real & _dt;
  Real _dt_old;

  /**
   * For storing all-purpose global params 
   */
  Parameters _pars;

  /**
   * List of systems being solved. Allocations/deallocations are responsibilities of derived classes
   */
  std::vector<System *> _sys;

  // Initial conditions
  std::map<std::string, InitialCondition *> _ics;

  // material properties
  MaterialProperties _material_props;
  MaterialProperties _material_props_old;
  MaterialProperties _material_props_older;

  // functions
  std::vector<std::map<std::string, Function *> > _functions;
};


void initial_condition(EquationSystems& es, const std::string& system_name);

} // namespace

#endif /* SUBPROBLEM_H_ */
