/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

#pragma once

#include "MultiAppTransfer.h"

class InterWrapperMesh;

/**
 * Base class for transfering solutions from computational mesh onto visualization mesh
 */
class InterWrapperSolutionTransferBase : public MultiAppTransfer
{
public:
  InterWrapperSolutionTransferBase(const InputParameters & parameters);

  virtual void execute() override;

protected:
  /**
   * Do the transfer into the sub-app
   */
  void transferToMultiApps();

  /**
   * Transfer variables into the sub-app
   *
   * @param app_idx Multi-app index
   */
  void transferVarsToApp(unsigned int app_idx);

  void transferNodalVars(unsigned int app_idx);

  /**
   * Find node on computational mesh given the visualization point
   *
   * @return Node from the computational mesh
   * @param from_mesh Computational mesh
   * @param src_node Node from the visualization
   */
  virtual Node * getFromNode(const InterWrapperMesh & from_mesh, const Point & src_node) = 0;

  /// Variable names to transfer
  const std::vector<AuxVariableName> & _var_names;

public:
  static InputParameters validParams();
};
