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

#ifndef MOOSE_H
#define MOOSE_H

#ifdef LIBMESH_HAVE_PETSC
#include "PetscSupport.h"
#endif //LIBMESH_HAVE_PETSC

//libMesh includes
#include "perf_log.h"
#include "InputParameters.h"
#include "getpot.h"
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
class MooseSystem;
class Executioner;
template <typename T> class MooseArray;

namespace libMesh
{  
  class Mesh;
  class EquationSystems;
  class ExodusII_IO;
  class ErrorEstimator;
  class ErrorVector;
}

#define MAX_VARS 1000

/**
 * Types for Variable
 */
typedef unsigned int             VariableNumber;
typedef MooseArray<Real>         VariableValue;
typedef MooseArray<RealGradient> VariableGradient;
typedef MooseArray<RealTensor>   VariableSecond;
typedef Real                     PostprocessorValue;
typedef unsigned int THREAD_ID;



#include <vector>

/**
 * These are here because of a problem with Parameter::print() for std::vectors
 */
namespace libMesh
{

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
  void InputParameters::Parameter<std::vector<std::vector<bool> > >::print (std::ostream& os) const
  {
    for (unsigned int i=0; i<_value.size(); i++)
      for (unsigned int j=0; i<_value[i].size(); j++)
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
  void InputParameters::Parameter<std::vector<bool> >::print (std::ostream& os) const
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

}

  
/**
 * A function to call when you need the whole program to die and spit out a message
 */
#ifndef NDEBUG
#define mooseError(msg) do { std::cerr << "\n\n" << msg << "\n\n"; print_trace(); libmesh_error(); } while(0)
#else
#define mooseError(msg) do { std::cerr << "\n\n" << msg << "\n\n"; libmesh_error(); } while(0)
#endif

#ifdef NDEBUG
#define mooseAssert(asserted, msg) 
#else
#define mooseAssert(asserted, msg)  do { if (!(asserted)) { std::cerr << "\n\nAssertion `" #asserted "' failed.\n" << msg << std::endl; libmesh_error(); } } while(0)
#endif

#define mooseWarning(msg) do { std::cerr << "\n\n*** Warning ***\n" << msg << "\nat " << __FILE__ << ", line " << __LINE__ << "\n" << std::endl; } while(0)


/**
 * likely() and unlikely() help the compiler optimize code by telling
 * it if an if statement is likely to be true or not.
 * the double negation !!(x) converts x to a 0 or 1 (like casting it to a bool)
 */
#ifdef __GNUC__
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#elif __INTEL_COMPILER                         // the intel compiler has the same builtin
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x) (x)
#define likely(x) (x)
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
  
  void setSolverDefaults(MooseSystem &moose_system, Executioner *executioner);
  
  void checkSystemsIntegrity();
  
  
  /*******************
   * Global Variables - yeah I know...
   *******************/

  enum GeomType
  {
    XYZ,
    CYLINDRICAL
  };

  enum TimeSteppingScheme
  {
    IMPLICIT_EULER,
    BDF2,
    CRANK_NICOLSON
  };

  // Bit mask flags to be able to combine them through or-operator (|)
  enum PostprocessorType
  {
    PPS_RESIDUAL = 0x01,
    PPS_JACOBIAN = 0x02,
    PPS_TIMESTEP = 0x04
  };

  const unsigned int ANY_BLOCK_ID = (unsigned int) -1;

  extern GetPot *command_line;
}

#endif //MOOSE_H
