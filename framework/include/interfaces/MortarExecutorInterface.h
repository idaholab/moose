//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

class FEProblemBase;
class MortarData;

/**
 * Interface for notifications that the mortar mesh has been setup
 */
class MortarExecutorInterface
{
public:
  MortarExecutorInterface(FEProblemBase & fe_problem);
  MortarExecutorInterface(MortarExecutorInterface && other);
  virtual ~MortarExecutorInterface() = default;

  /**
   * Called on this object when the mesh changes
   */
  virtual void mortarSetup() = 0;

private:
  MortarData & _mortar_data;
};
