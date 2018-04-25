//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef VECTORPOSTPROCESSORDATA_H
#define VECTORPOSTPROCESSORDATA_H

// MOOSE includes
#include "MooseTypes.h"
#include "Restartable.h"

#include <map>

class FEProblemBase;

class VectorPostprocessorData : public Restartable
{
public:
  /**
   * Class constructor
   */
  VectorPostprocessorData(FEProblemBase & fe_problem);

  /**
   * Initialization method, sets the current and old value to 0.0 for this postprocessor
   */
  void init(const std::string & name);

  /// VectorPostprocessorState (2 containers for values (see MooseTypes.h)
  struct VectorPostprocessorState
  {
    VectorPostprocessorValue * current;
    VectorPostprocessorValue * old;
  };

  /**
   * Initialization method, sets the current and old value to 0.0 for this
   * VectorPostprocessor
   *
   * @param vpp_name The name of the VectorPostprocessor
   * @param vector_name The name of the vector
   */
  VectorPostprocessorValue & declareVector(const std::string & vpp_name,
                                           const std::string & vector_name,
                                           bool contains_complete_history);

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
  VectorPostprocessorValue & getVectorPostprocessorValue(const VectorPostprocessorName & vpp_name,
                                                         const std::string & vector_name);

  /**
   * The the old value of an post-processor
   * @param vpp_name The name of the VectorPostprocessor
   * @param vector_name The name of the vector
   * @return The reference to the old value
   */
  VectorPostprocessorValue &
  getVectorPostprocessorValueOld(const VectorPostprocessorName & vpp_name,
                                 const std::string & vector_name);

  /**
   * Check to see if a VPP has any vectors at all
   */
  bool hasVectors(const std::string & vpp_name) const;

  /**
   * Returns a Boolean indicating whether the specified VPP vectors contain complete history.
   */
  bool containsCompleteHistory(const std::string & name) const;

  /**
   * Get the map of vectors for a particular VectorPostprocessor
   * @param vpp_name The name of the VectorPostprocessor
   */
  const std::vector<std::pair<std::string, VectorPostprocessorState>> &
  vectors(const std::string & vpp_name) const;

  /**
   * Copy the current post-processor values into old (i.e. shift it "back in time") as needed
   */
  void copyValuesBack();

private:
  VectorPostprocessorValue & getVectorPostprocessorHelper(const VectorPostprocessorName & vpp_name,
                                                          const std::string & vector_name,
                                                          bool get_current = true,
                                                          bool contains_complete_history = false);
  /**
   * Vector of pairs representing the declared vectors (vector name, vector DS)
   * The vector DS is a data structure containing a current and old container (vector of Reals)
   */
  struct VectorPostprocessorVectors
  {
    VectorPostprocessorVectors();

    ///@{
    // Default move constructors
    VectorPostprocessorVectors(VectorPostprocessorVectors &&) = default;
    VectorPostprocessorVectors & operator=(VectorPostprocessorVectors &&) = default;
    ///@}

    std::vector<std::pair<std::string, VectorPostprocessorState>> _values;

    /// Boolean indicating whether these vectors contain complete history (append mode)
    bool _contains_complete_history;

    /// Boolean indicating whether any old vectors have been requested.
    bool _needs_old;
  };

  /// The VPP data store in a map: VPP Name to vector storage
  std::map<std::string, VectorPostprocessorVectors> _vpp_data;

  std::set<std::string> _requested_items;
  std::set<std::string> _supplied_items;
};

#endif // VECTORPOSTPROCESSORDATA_H
