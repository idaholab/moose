#ifndef MOOSE_H
#define MOOSE_H

#ifdef LIBMESH_HAVE_PETSC
#include "PetscSupport.h"
#endif //LIBMESH_HAVE_PETSC

//libMesh includes
#include "perf_log.h"
#include "InputParameters.h"
#include "getpot.h"
#include "elem_range.h"
#include "vector_value.h"
#include "libmesh.h"
#include "mesh.h"
#include "equation_systems.h"
#include "nonlinear_solver.h"
#include "nonlinear_implicit_system.h"
#include "transient_system.h"
#include "preconditioner.h"
#include "print_trace.h"

//Forward Declarations
class Mesh;
class EquationSystems;
class ExodusII_IO;
class ErrorEstimator;
class ErrorVector;
class Executioner;
class MooseSystem;

#define MAX_VARS 1000



typedef unsigned int THREAD_ID;



#include <vector>

/**
 * These are here because of a problem with Parameter::print() for std::vectors
 */
template<>
inline
void InputParameters::Parameter<std::vector<Real> >::print (std::ostream& os) const
{
  for (unsigned int i=0; i<_value.size(); i++)
    os << _value[i] << " ";
}

template<>
inline
void InputParameters::Parameter<std::vector<unsigned int> >::print (std::ostream& os) const
{
  for (unsigned int i=0; i<_value.size(); i++)
    os << _value[i] << " ";
}

template<>
inline
void InputParameters::Parameter<std::vector<std::vector<Real> > >::print (std::ostream& os) const
{
  for (unsigned int i=0; i<_value.size(); i++)
    for (unsigned int j=0; i<_value[i].size(); j++)
      os << _value[i][j] << " ";
}

template<>
inline
void InputParameters::Parameter<std::vector<int> >::print (std::ostream& os) const
{
  for (unsigned int i=0; i<_value.size(); i++)
    os << _value[i] << " ";
}

template<>
inline
void InputParameters::Parameter<std::vector<std::vector<int> > >::print (std::ostream& os) const
{
  for (unsigned int i=0; i<_value[i].size(); i++)
    for (unsigned int j=0; i<_value[j].size(); j++)
      os << _value[i][j] << " ";
}
 
template<>
inline
void InputParameters::Parameter<std::vector<std::string> >::print (std::ostream& os) const
{
  for (unsigned int i=0; i<_value.size(); i++)
    os << _value[i] << " ";
}

template<>
inline
void InputParameters::Parameter<GetPot>::print (std::ostream& /*os*/) const
{
}

template<>
inline
void InputParameters::Parameter<std::vector<float> >::print (std::ostream& os) const
{
  for (unsigned int i=0; i<_value.size(); i++)
    os << _value[i] << " ";
}

template<>
inline
void InputParameters::Parameter<std::map<std::string, unsigned int> >::print (std::ostream& /*os*/) const
{
}

/**
 * A function to call when you need the whole program to die a spit out a message
 */

#ifndef NDEBUG
#define mooseError(msg) do { std::cerr << "\n\n" << msg << "\n\n"; print_trace(); libmesh_error(); } while(0)
#else
#define mooseError(msg) do { std::cerr << "\n\n" << msg << "\n\n"; libmesh_error(); } while(0)
#endif

#ifdef NDEBUG
#define mooseAssert(asserted, msg) 
#else
#define mooseAssert(asserted, msg)  do { if (!(asserted)) { std::cerr << "Assertion `" #asserted "' failed.\n" << msg << std::endl; libmesh_error(); } } while(0)
#endif


class MooseInit : public LibMeshInit
{
public:
  MooseInit(int argc, char** argv);

  virtual ~MooseInit();
};

namespace Moose
{
  /**
   * Perflog to be used by applications.
   * If the application prints this in the end they will get performance info.
   */
  extern PerfLog perf_log;

  /**
   * Registers the Kernels and BCs provided by Moose.
   */
  void registerObjects();

  /**
   * Initialize MOOSE and the underlying Libmesh libraries and command line parser
   */
  void initMoose(int argc, char** argv);
  
  void setSolverDefaults(EquationSystems * es,
                         TransientNonlinearImplicitSystem & system,
                         void (*compute_jacobian_block) (const NumericVector<Number>& soln, SparseMatrix<Number>&  jacobian, System& precond_system, NonlinearImplicitSystem& sys, unsigned int ivar, unsigned int jvar),
                         void (*compute_residual) (const NumericVector<Number>& soln, NumericVector<Number>& residual, NonlinearImplicitSystem& sys));
  
  void checkSystemsIntegrity();
  
  
  /*******************
   * Global Variables - yeah I know...
   *******************/


  /**
   * Current thread id... this is used by serial processes to set stuff up.
   * This is NOT valid inside of a thread!
   */
  extern THREAD_ID current_thread_id;

  /**
   * A mesh refinement object to be used with Adaptivity.
   */
  extern MeshRefinement * mesh_refinement;

  /**
   * Error estimator to be used by the apps.
   */
  extern ErrorEstimator * error_estimator;

  /**
   * Error vector for use with the error estimator.
   */
  extern ErrorVector * error;

  /**
   * THE Executioner object that will be used.
   */
  extern Executioner * executioner;

  enum GeomType
  {
    XYZ,
    CYLINDRICAL
  };

  /**
   * If this is true than the finite element objects will only get reinited _once_!
   *
   * This is only valid if you are using a perfectly regular grid!
   *
   * This can provide a huge speedup... but must be used with care.
   */
  extern bool no_fe_reinit;

  extern std::string execution_type;

  extern std::string file_base;
  extern int interval;
  extern bool exodus_output;
  extern bool gmv_output;
  extern bool tecplot_output;
  extern bool print_out_info;
  extern bool output_initial;
  extern bool auto_scaling;

  extern std::vector<Real> manual_scaling;

  extern GetPot *command_line;
}

/**
 * Add a non-existent value into std;:vector
 *
 * A helper function that searches the vector for value 'value' and adds it, if it does not exist yet
 */
template<class T, class EqualityComparable>
void add_nonexistent(const EqualityComparable& value, std::vector<T> &vec)
{
  if(std::find(vec.begin(), vec.end(), value) == vec.end())
    vec.push_back(value);
}

#endif //MOOSE_H
