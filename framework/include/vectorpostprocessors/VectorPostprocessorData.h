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

#ifndef VECTORPOSTPROCESSORDATA_H
#define VECTORPOSTPROCESSORDATA_H

//MOOSE includes
#include "MooseTypes.h"
#include "Restartable.h"

#include <map>

class FEProblem;

class VectorPostprocessorData : public Restartable
{
public:
  /**
   * Class constructor
   */
  VectorPostprocessorData(FEProblem & fe_problem);

  /**
   * Initialization method, sets the current and old value to 0.0 for this
   * VectorPostprocessor
   *
   * @param vpp_name The name of the VectorPostprocessor
   * @param vector_name The name of the vector
   */
  VectorPostprocessorValue & declareVector(const std::string & vpp_name, const std::string & vector_name);

  /**
   * Returns a true value if the VectorPostprocessor exists
   */
  bool hasVectorPostprocessor(const std::string & name);

  /**
   * Return the value for the post processor
   * @param vpp_name The name of the VectorPostprocessor
   * @param vector_name The name of the vector
   * @return The reference to the current value
   */
  VectorPostprocessorValue & getVectorPostprocessorValue(const VectorPostprocessorName & vpp_name, const std::string & vector_name);

  /**
   * The the old value of an post-processor
   * @param vpp_name The name of the VectorPostprocessor
   * @param vector_name The name of the vector
   * @return The reference to the old value
   */
  VectorPostprocessorValue & getVectorPostprocessorValueOld(const VectorPostprocessorName & vpp_name, const std::string & vector_name);

  /**
   * Get the map of names -> VectorPostprocessor values. Exposed for error checking.
   */
  const std::map<std::string, std::map<std::string, VectorPostprocessorValue*> > & values() const { return _values; }

  /**
   * Get the map of vectors for a particular VectorPostprocessor
   * @param vpp_name The name of the VectorPostprocessor
   */
  const std::map<std::string, VectorPostprocessorValue*> & vectors(const std::string & vpp_name) { return _values[vpp_name]; }

  /**
   * Copy the current post-processor values into old (i.e. shift it "back in time")
   */
  void copyValuesBack();

protected:

  /// Values of the post-processor at the current time
  std::map<std::string, std::map<std::string, VectorPostprocessorValue*> > _values;

  /// Values of the post-processors at the time t-1
  std::map<std::string, std::map<std::string, VectorPostprocessorValue*> > _values_old;
};

#endif //VECTORPOSTPROCESSORDATA_H
