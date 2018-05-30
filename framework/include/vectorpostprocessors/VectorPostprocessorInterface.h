//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef VECTORPOSTPROCESSORINTERFACE_H
#define VECTORPOSTPROCESSORINTERFACE_H

// MOOSE includes
#include "MooseTypes.h"

// Forward Declarations
class FEProblemBase;
class InputParameters;
class MooseObject;

class VectorPostprocessorInterface
{
public:
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
   * @param name The name of the VectorPostprocessor parameter (see below)
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
  virtual const VectorPostprocessorValue &
  getVectorPostprocessorValue(const std::string & name, const std::string & vector_name);

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
  virtual const VectorPostprocessorValue &
  getVectorPostprocessorValueByName(const VectorPostprocessorName & name,
                                    const std::string & vector_name);

  /**
   * DEPRECATED: Use the new version where you need to specify whether or
   * not the vector must be broadcast
   *
   * Retrieve the old value of a VectorPostprocessor
   * @param name The name of the VectorPostprocessor parameter
   * @param vector_name The name of the particular vector you want.
   * @return The value of the VectorPostprocessor
   *
   * see getVectorPostprocessorValue
   */
  const VectorPostprocessorValue & getVectorPostprocessorValueOld(const std::string & name,
                                                                  const std::string & vector_name);

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
                                       const std::string & vector_name);

  /**
   * Retrieve the value of a VectorPostprocessor
   * @param name The name of the VectorPostprocessor parameter (see below)
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
  virtual const VectorPostprocessorValue & getVectorPostprocessorValue(
      const std::string & name, const std::string & vector_name, bool needs_broadcast);

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
  virtual const VectorPostprocessorValue & getVectorPostprocessorValueByName(
      const VectorPostprocessorName & name, const std::string & vector_name, bool needs_broadcast);

  /**
   * Retrieve the old value of a VectorPostprocessor
   * @param name The name of the VectorPostprocessor parameter
   * @param vector_name The name of the particular vector you want.
   * @param need_broadcast Whether or not this object requires the vector to
   * be replicated in parallel
   * @return The value of the VectorPostprocessor
   *
   * see getVectorPostprocessorValue
   */
  virtual const VectorPostprocessorValue & getVectorPostprocessorValueOld(
      const std::string & name, const std::string & vector_name, bool needs_broadcast);

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
  virtual const VectorPostprocessorValue & getVectorPostprocessorValueOldByName(
      const VectorPostprocessorName & name, const std::string & vector_name, bool needs_broadcast);

  /**
   * Return the scatter value for the post processor
   *
   * This is only valid when you expec the vector to be of lenghth "num_procs"
   * In that case - this will return a reference to a value that will be _this_ processor's value
   * from that vector
   *
   * @param name The name of the parameter holding the vpp name
   * @param vector_name The name of the vector
   * @return The reference to the current scatter value
   */
  virtual const ScatterVectorPostprocessorValue &
  getScatterVectorPostprocessorValue(const std::string & name, const std::string & vector_name);

  /**
   * Return the scatter value for the post processor
   *
   * This is only valid when you expec the vector to be of lenghth "num_procs"
   * In that case - this will return a reference to a value that will be _this_ processor's value
   * from that vector
   *
   * @param vpp_name The name of the VectorPostprocessor
   * @param vector_name The name of the vector
   * @return The reference to the current scatter value
   */
  virtual const ScatterVectorPostprocessorValue &
  getScatterVectorPostprocessorValueByName(const std::string & name,
                                           const std::string & vector_name);

  /**
   * Return the old scatter value for the post processor
   *
   * This is only valid when you expec the vector to be of lenghth "num_procs"
   * In that case - this will return a reference to a value that will be _this_ processor's
   * value from that vector
   *
   * @param name The name of the parameter holding the vpp name
   * @param vector_name The name of the vector
   * @return The reference to the old scatter value
   */
  virtual const ScatterVectorPostprocessorValue &
  getScatterVectorPostprocessorValueOld(const std::string & name, const std::string & vector_name);

  /**
   * Return the old scatter value for the post processor
   *
   * This is only valid when you expec the vector to be of lenghth "num_procs"
   * In that case - this will return a reference to a value that will be _this_ processor's
   * value from that vector
   *
   * @param vpp_name The name of the VectorPostprocessor
   * @param vector_name The name of the vector
   * @return The reference to the old scatter value
   */
  virtual const ScatterVectorPostprocessorValue &
  getScatterVectorPostprocessorValueOldByName(const std::string & name,
                                              const std::string & vector_name);

  /**
   * Determine if the VectorPostprocessor exists
   * @param name The name of the VectorPostprocessor parameter
   * @return True if the VectorPostprocessor exists
   *
   * @see hasVectorPostprocessorByName getVectorPostprocessorValue
   */
  bool hasVectorPostprocessor(const std::string & name) const;

  /**
   * Determine if the VectorPostprocessor exists
   * @param name The name of the VectorPostprocessor
   * @return True if the VectorPostprocessor exists
   *
   * @see hasVectorPostprocessor getVectorPostprocessorValueByName
   */
  bool hasVectorPostprocessorByName(const VectorPostprocessorName & name) const;

private:
  /// Whether or not to force broadcasting by default
  bool _broadcast_by_default;

  /// VectorPostprocessorInterface Parameters
  const InputParameters & _vpi_params;

  /// Reference the the FEProblemBase class
  FEProblemBase & _vpi_feproblem;

  /// Thread ID
  THREAD_ID _vpi_tid;
};

#endif // VECTORPOSTPROCESSORINTERFACE_H
