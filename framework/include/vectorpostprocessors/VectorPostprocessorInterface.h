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

#ifndef VECTORPOSTPROCESSORINTERFACE_H
#define VECTORPOSTPROCESSORINTERFACE_H

// MOOSE includes
#include "InputParameters.h"
#include "ParallelUniqueId.h"

// Forward Declarations
class FEProblemBase;
class MooseObject;

class VectorPostprocessorInterface
{
public:
  VectorPostprocessorInterface(const MooseObject * moose_object);

  /**
   * This class has virtual methods, so it needs a virtual dtor.
   */
  virtual ~VectorPostprocessorInterface() = default;

  /**
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
  /// VectorPostprocessorInterface Parameters
  const InputParameters & _vpi_params;

  /// Reference the the FEProblemBase class
  FEProblemBase & _vpi_feproblem;

  /// Thread ID
  THREAD_ID _vpi_tid;
};

#endif // VECTORPOSTPROCESSORINTERFACE_H
