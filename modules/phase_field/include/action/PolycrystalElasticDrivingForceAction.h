/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POLYCRYSTALELASTICDRIVINGFORCEACTION_H
#define POLYCRYSTALELASTICDRIVINGFORCEACTION_H

#include "Action.h"
#include "DerivativeMaterialPropertyNameInterface.h"

// Forward Declarations
class PolycrystalElasticDrivingForceAction;

template <>
InputParameters validParams<PolycrystalElasticDrivingForceAction>();
/**
 * Action that adds the elastic driving force for each order parameter
 */
class PolycrystalElasticDrivingForceAction : public Action,
                                             public DerivativeMaterialPropertyNameInterface
{
public:
  PolycrystalElasticDrivingForceAction(const InputParameters & params);

  virtual void act();

private:
  /// Number of order parameters used in the model
  const unsigned int _op_num;

  /// Base name for the order parameters
  std::string _var_name_base;
  std::string _base_name;
  std::string _elasticity_tensor_name;
};

#endif // POLYCRYSTALELASTICDRIVINGFORCEACTION_H
