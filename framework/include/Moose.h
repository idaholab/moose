#ifndef MOOSE_H
#define MOOSE_H
//libMesh includes
#include "perf_log.h"
#include "parameters.h"
#include "getpot.h"
#include "elem_range.h"

//Forward Declarations
class Mesh;
class EquationSystems;
class ExodusII_IO;

#define MAX_VARS 1000



typedef unsigned int THREAD_ID;



#include <vector>

/**
 * These are here because of a problem with Parameter::print() for std::vectors
 */
template<>
inline
void Parameters::Parameter<std::vector<Real> >::print (std::ostream& os) const
{
  for (unsigned int i=0; i<_value.size(); i++)
    os << _value[i] << " ";
}

template<>
inline
void Parameters::Parameter<std::vector<std::vector<Real> > >::print (std::ostream& os) const
{
  for (unsigned int i=0; i<_value[i].size(); i++)
    for (unsigned int j=0; i<_value[j].size(); j++)
      os << _value[i][j] << " ";
}

template<>
inline
void Parameters::Parameter<std::vector<int> >::print (std::ostream& os) const
{
  for (unsigned int i=0; i<_value.size(); i++)
    os << _value[i] << " ";
}

template<>
inline
void Parameters::Parameter<std::vector<std::vector<int> > >::print (std::ostream& os) const
{
  for (unsigned int i=0; i<_value[i].size(); i++)
    for (unsigned int j=0; i<_value[j].size(); j++)
      os << _value[i][j] << " ";
}
 
template<>
inline
void Parameters::Parameter<std::vector<std::string> >::print (std::ostream& os) const
{
  for (unsigned int i=0; i<_value.size(); i++)
    os << _value[i] << " ";
}

template<>
inline
void Parameters::Parameter<GetPot>::print (std::ostream& os) const
{
}

template<>
inline
void Parameters::Parameter<std::vector<float> >::print (std::ostream& os) const
{
  for (unsigned int i=0; i<_value.size(); i++)
    os << _value[i] << " ";
}

template<>
inline
void Parameters::Parameter<std::map<std::string, unsigned int> >::print (std::ostream& os) const
{
}

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
   * Should be called after the mesh has been modified in any way.
   */
  void meshChanged();


  /**
   * Get access to the active_local_element_range
   * Automatically builds it if it hasn't been initialized.
   */
  ConstElemRange * getActiveLocalElementRange();
  
  /*******************
   * Global Variables - yeah I know...
   *******************/


  /**
   * Current thread id... this is used by serial processes to set stuff up.
   * This is NOT valid inside of a thread!
   */
  extern THREAD_ID current_thread_id;
  
  /**
   * The one mesh to rule them all
   */
  extern Mesh * mesh;

  /**
   * The ExodusIO Reader to support reading of solutions at element qps
   */
  extern ExodusII_IO * exreader;

  /**
   * The one equation system to rule them all
   */
  extern EquationSystems * equation_system;

  /**
   * A range for use with TBB.  We do this so that it doesn't have
   * to get rebuilt all the time (which takes time).
   */
  extern ConstElemRange * active_local_elem_range;

  enum GeomType
  {
    XYZ,
    CYLINDRICAL
  };

  extern GeomType geom_type;

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

  extern MeshRefinement * mesh_refinement;
  extern std::vector<Real> manual_scaling;

}

#endif //MOOSE_H
