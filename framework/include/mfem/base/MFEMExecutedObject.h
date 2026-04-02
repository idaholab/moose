//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "MFEMObject.h"
#include "SetupInterface.h"

#include <set>

/**
 * Base class for MFEM objects that participate in execution ordering but are not UserObjects.
 */
class MFEMExecutedObject : public MFEMObject, public SetupInterface
{
public:
  static InputParameters validParams();

  MFEMExecutedObject(const InputParameters & parameters);

  virtual void initialize() {}
  virtual void execute() {}
  virtual void finalize() {}

  virtual std::set<std::string> consumedVariableNames() const;
  virtual std::set<std::string> producedVariableNames() const;
  virtual std::set<std::string> consumedPostprocessorNames() const;
  virtual std::set<std::string> producedPostprocessorNames() const;
  virtual std::set<std::string> consumedVectorPostprocessorNames() const;
  virtual std::set<std::string> producedVectorPostprocessorNames() const;

protected:
  template <typename T>
  void appendTypedParamIfValid(std::set<std::string> & names, const std::string & param_name) const;

  template <typename T>
  void appendTypedVectorParamIfValid(std::set<std::string> & names,
                                     const std::string & param_name) const;
};

#endif
