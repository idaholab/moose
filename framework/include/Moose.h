//libMesh includes
#include "perf_log.h"
#include "parameters.h"
#include "getpot.h"

//Forward Declarations
class Mesh;
class EquationSystems;

#define MAX_VARS 1000

#ifndef MOOSE_H
#define MOOSE_H

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


  /*******************
   * Global Variables - yeah I know...
   *******************/

  /**
   * The one mesh to rule them all
   */
  extern Mesh * mesh;

  /**
   * The one equation system to rule them all
   */
  extern EquationSystems * equation_system;

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
}

#endif //MOOSE_H
