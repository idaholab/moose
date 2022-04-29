/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*   Pronghorn: Coarse-Mesh, Multi-Dimensional, Thermal-Hydraulics  */
/*                                                                  */
/*              (c) 2020 Battelle Energy Alliance, LLC              */
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
#include "Coupleable.h"
/**
 * Reconstruct a CONSTANT MONOMIAL variable in the master app assigning the
 * closest Multiapp postprocessor value to each element.
 */
class MultiAppPostprocessorNearestElementTransfer : public MultiAppTransfer
{
public:
  static InputParameters validParams();

  MultiAppPostprocessorNearestElementTransfer(const InputParameters & parameters);

  virtual void initialSetup() override;

  virtual void execute() override;

protected:
  AuxVariableName _postprocessor_name;
  PostprocessorName _to_var_name;
  const Real _relax_factor;

  // References to the problem, the variable and the system.
  FEProblemBase & _to_problem;
  MooseVariableFieldBase & _to_var;
  System & _to_sys;

  // Mesh, system number and variable number to check if the variable has dofs
  // in the element.
  MeshBase & _to_mesh;
  unsigned int _to_sys_num;
  unsigned int _to_var_num;

  // Element closest multiapp vector map.
  std::vector<unsigned int> _cached_multiapp_pos_ids;
};
