//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MooseTypes.h"

// Forward Declarations
class FEProblemBase;
class InputParameters;
class MooseObject;
template <typename T>
class VectorPostprocessorContext;

class VectorPostprocessorInterface
{
public:
  static InputParameters validParams();

  /**
   * Constructor
   *
   * @param broadcast_by_default Set to true if the system inheriting from this interface always
   * needs the VPPs to be broadcast
   */
  VectorPostprocessorInterface(const MooseObject * moose_object, bool broadcast_by_default = false);

  /**
   * This class has virtual methods, so it needs a virtual dtor.
   */
  virtual ~VectorPostprocessorInterface() = default;

  /**
   * DEPRECATED: Use the new version where you need to specify whether or
   * not the vector must be broadcast
   *
   * Retrieve the value of a VectorPostprocessor
   * @param param_name The name of the VectorPostprocessor parameter (see below)
   * @param vector_name The name of the particular vector you want.
   * @return A reference to the desired value
   *
   * The name required by this method is the name that is hard-coded into
   * your source code. For example, if you have a Kernel that requires
   * a VectorPostprocessor you may have an input file with "pp = my_pp", this function
   * requires the "pp" name as input (see .../moose_test/functions/VectorPostprocessorFunction.C)
   *
   * see getVectorPostprocessorValueOld getVectorPostprocessorValueByName
   * getVectorPostprocessorValueOldByName
   */
  const VectorPostprocessorValue &
  getVectorPostprocessorValue(const std::string & param_name,
                              const std::string & vector_name) const;

  /**
   * DEPRECATED: Use the new version where you need to specify whether or
   * not the vector must be broadcast
   *
   * Retrieve the value of the VectorPostprocessor
   * @param name VectorPostprocessor name (see below)
   * @param vector_name The name of the particular vector you want.
   * @return A reference to the desired value
   *
   * The name required by this method is the name defined in the input file. For example,
   * if you have a Kernel that requires a VectorPostprocessor you may have an input file with
   * "pp = my_pp", this method requires the "my_pp" name as input
   * (see .../moose_test/functions/VectorPostprocessorFunction.C)
   *
   * see getVectorPostprocessorValue getVectorPostprocessorValueOldByName
   * getVectorPostprocessorValueByName
   */
  const VectorPostprocessorValue &
  getVectorPostprocessorValueByName(const VectorPostprocessorName & name,
                                    const std::string & vector_name) const;

  /**
   * DEPRECATED: Use the new version where you need to specify whether or
   * not the vector must be broadcast
   *
   * Retrieve the old value of a VectorPostprocessor
   * @param param_name The name of the VectorPostprocessor parameter
   * @param vector_name The name of the particular vector you want.
   * @return The value of the VectorPostprocessor
   *
   * see getVectorPostprocessorValue
   */
  const VectorPostprocessorValue &
  getVectorPostprocessorValueOld(const std::string & param_name,
                                 const std::string & vector_name) const;

  /**
   * DEPRECATED: Use the new version where you need to specify whether or
   * not the vector must be broadcast
   *
   * Retrieve the old value of a VectorPostprocessor
   * @param name The name of the VectorPostprocessor
   * @param vector_name The name of the particular vector you want.
   * @return The value of the VectorPostprocessor
   *
   * If within the validParams for the object the addVectorPostprocessorParam was called this method
   * will retun a reference to the default value specified in the call to the
   * addVectorPostprocessorParam
   * function if the postVectorPostprocessor does not exist.
   *
   * see getVectorPostprocessorValueByName
   */
  const VectorPostprocessorValue &
  getVectorPostprocessorValueOldByName(const VectorPostprocessorName & name,
                                       const std::string & vector_name) const;

  // doco-normal-methods-begin
  /**
   * Retrieve the value of a VectorPostprocessor
   * @param param_name The name of the VectorPostprocessor parameter (see below)
   * @param vector_name The name of the particular vector you want.
   * @param need_broadcast Whether or not this object requires the vector to
   * be replicated in parallel
   * @return A reference to the desired value
   *
   * The name required by this method is the name that is hard-coded into
   * your source code. For example, if you have a Kernel that requires
   * a VectorPostprocessor you may have an input file with "pp = my_pp", this function
   * requires the "pp" name as input (see .../moose_test/functions/VectorPostprocessorFunction.C)
   *
   * see getVectorPostprocessorValueOld getVectorPostprocessorValueByName
   * getVectorPostprocessorValueOldByName
   */
  const VectorPostprocessorValue & getVectorPostprocessorValue(const std::string & param_name,
                                                               const std::string & vector_name,
                                                               bool needs_broadcast) const;
  // doco-normal-methods-end

  /**
   * Retrieve the value of the VectorPostprocessor
   * @param name VectorPostprocessor name (see below)
   * @param vector_name The name of the particular vector you want.
   * @param need_broadcast Whether or not this object requires the vector to
   * be replicated in parallel
   * @return A reference to the desired value
   *
   * The name required by this method is the name defined in the input file. For example,
   * if you have a Kernel that requires a VectorPostprocessor you may have an input file with
   * "pp = my_pp", this method requires the "my_pp" name as input
   * (see .../moose_test/functions/VectorPostprocessorFunction.C)
   *
   * see getVectorPostprocessorValue getVectorPostprocessorValueOldByName
   * getVectorPostprocessorValueByName
   */
  const VectorPostprocessorValue &
  getVectorPostprocessorValueByName(const VectorPostprocessorName & name,
                                    const std::string & vector_name,
                                    bool needs_broadcast) const;

  /**
   * Retrieve the old value of a VectorPostprocessor
   * @param param_name The name of the VectorPostprocessor parameter
   * @param vector_name The name of the particular vector you want.
   * @param need_broadcast Whether or not this object requires the vector to
   * be replicated in parallel
   * @return The value of the VectorPostprocessor
   *
   * see getVectorPostprocessorValue
   */
  const VectorPostprocessorValue & getVectorPostprocessorValueOld(const std::string & param_name,
                                                                  const std::string & vector_name,
                                                                  bool needs_broadcast) const;

  /**
   * Retrieve the old value of a VectorPostprocessor
   * @param name The name of the VectorPostprocessor
   * @param vector_name The name of the particular vector you want.
   * @param need_broadcast Whether or not this object requires the vector to
   * be replicated in parallel
   * @return The value of the VectorPostprocessor
   *
   * If within the validParams for the object the addVectorPostprocessorParam was called this method
   * will retun a reference to the default value specified in the call to the
   * addVectorPostprocessorParam
   * function if the postVectorPostprocessor does not exist.
   *
   * see getVectorPostprocessorValueByName
   */
  const VectorPostprocessorValue &
  getVectorPostprocessorValueOldByName(const VectorPostprocessorName & name,
                                       const std::string & vector_name,
                                       bool needs_broadcast) const;

  /**
   * Return the scatter value for the post processor
   *
   * This is only valid when you expec the vector to be of lenghth "num_procs"
   * In that case - this will return a reference to a value that will be _this_ processor's value
   * from that vector
   *
   * @param param_name The name of the parameter holding the vpp name
   * @param vector_name The name of the vector
   * @return The reference to the current scatter value
   */
  const ScatterVectorPostprocessorValue &
  getScatterVectorPostprocessorValue(const std::string & param_name,
                                     const std::string & vector_name) const;

  /**
   * Return the scatter value for the post processor
   *
   * This is only valid when you expec the vector to be of lenghth "num_procs"
   * In that case - this will return a reference to a value that will be _this_ processor's value
   * from that vector
   *
   * @param name The name of the VectorPostprocessor
   * @param vector_name The name of the vector
   * @return The reference to the current scatter value
   */
  const ScatterVectorPostprocessorValue &
  getScatterVectorPostprocessorValueByName(const VectorPostprocessorName & name,
                                           const std::string & vector_name) const;

  /**
   * Return the old scatter value for the post processor
   *
   * This is only valid when you expec the vector to be of lenghth "num_procs"
   * In that case - this will return a reference to a value that will be _this_ processor's
   * value from that vector
   *
   * @param param_name The name of the parameter holding the vpp name
   * @param vector_name The name of the vector
   * @return The reference to the old scatter value
   */
  const ScatterVectorPostprocessorValue &
  getScatterVectorPostprocessorValueOld(const std::string & param_name,
                                        const std::string & vector_name) const;

  /**
   * Return the old scatter value for the post processor
   *
   * This is only valid when you expect the vector to be of length "num_procs"
   * In that case - this will return a reference to a value that will be _this_ processor's
   * value from that vector
   *
   * @param name The name of the VectorPostprocessor
   * @param vector_name The name of the vector
   * @return The reference to the old scatter value
   */
  const ScatterVectorPostprocessorValue &
  getScatterVectorPostprocessorValueOldByName(const VectorPostprocessorName & name,
                                              const std::string & vector_name) const;

  /**
   * Determine if the VectorPostprocessor data exists by parameter
   * @param param_name The name of the VectorPostprocessor parameter
   * @param vector_name The vector name within the VectorPostprocessor
   * @return True if the VectorPostprocessor data exists
   *
   * @see hasVectorPostprocessorByName getVectorPostprocessorValue
   */
  bool hasVectorPostprocessor(const std::string & param_name,
                              const std::string & vector_name) const;

  /**
   * Determine if the VectorPostprocessor data exists by name
   * @param name The name of the VectorPostprocessor
   * @param vector_name The vector name within the VectorPostprocessor
   * @return True if the VectorPostprocessor data exists
   *
   * @see hasVectorPostprocessor getVectorPostprocessorValueByName
   */
  bool hasVectorPostprocessorByName(const VectorPostprocessorName & name,
                                    const std::string & vector_name) const;

  /**
   * Determine if the VectorPostprocessor exists by parameter
   * @param name The name of the VectorPostprocessor parameter
   * @return True if the VectorPostprocessor exists
   */
  bool hasVectorPostprocessor(const std::string & param_name) const;

  /**
   * Determine if the VectorPostprocessor exists by name
   * @param name The name of the VectorPostprocessor
   * @return True if the VectorPostprocessor exists
   */
  bool hasVectorPostprocessorByName(const VectorPostprocessorName & name) const;

  ///@{
  /**
   * Return true if the VectorPostprocessor is marked with parallel_type as DISTRIBUTED
   */
  bool isVectorPostprocessorDistributed(const std::string & param_name) const;
  bool isVectorPostprocessorDistributedByName(const VectorPostprocessorName & name) const;
  ///@}

  /**
   * Get the name of a VectorPostprocessor associated with a parameter.
   * @param param_name The name of the VectorPostprocessor parameter
   * @return The name of the given VectorPostprocessor
   */
  const VectorPostprocessorName & getVectorPostprocessorName(const std::string & param_name) const;

protected:
  /**
   * Helper for deriving classes to override to add dependencies when a VectorPostprocessor is
   * requested.
   */
  virtual void
  addVectorPostprocessorDependencyHelper(const VectorPostprocessorName & /* name */) const
  {
  }

private:
  /**
   * Helper function for extracting VPP data from ReporterData object
   */
  const VectorPostprocessorValue &
  getVectorPostprocessorByNameHelper(const VectorPostprocessorName & name,
                                     const std::string & vector_name,
                                     bool broadcast,
                                     std::size_t t_index) const;

  /**
   * Helper for getting the VPP context that handles scatter values
   */
  const VectorPostprocessorContext<VectorPostprocessorValue> &
  getVectorPostprocessorContextByNameHelper(const VectorPostprocessorName & name,
                                            const std::string & vector_name) const;

  ///@{
  /**
   * Helpers for "possibly" checking if a vpp exists. This is only able to check for
   * existance after all vpps have been added (after the task creating them has
   * been called). If called before said task, this will do nothing, hence the
   * "possibly". This allows us to have errors reported directly by the object
   * requesting the vpp instead of through a system with less context.
   */
  void possiblyCheckHasVectorPostprocessor(const std::string & param_name,
                                           const std::string & vector_name) const;
  void possiblyCheckHasVectorPostprocessorByName(const VectorPostprocessorName & name,
                                                 const std::string & vector_name) const;
  ///@}

  /**
   * @returns True if all vpps have been added (the task associated with adding them is complete)
   */
  bool vectorPostprocessorsAdded() const;

  /// Whether or not to force broadcasting by default
  const bool _broadcast_by_default;

  /// The MooseObject that uses this interface
  const MooseObject & _vpi_moose_object;

  /// Reference the FEProblemBase class
  const FEProblemBase & _vpi_feproblem;

  /// Thread ID
  const THREAD_ID _vpi_tid;
};
