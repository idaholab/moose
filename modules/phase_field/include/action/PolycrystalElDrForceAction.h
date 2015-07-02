#ifndef POLYCRYSTALELDRFORCEACTION_H
#define POLYCRYSTALELDRFORCEACTION_H

#include "Action.h"

// Forward Declarations
class PolycrystalElDrForceAction;

template<>
InputParameters validParams<PolycrystalElDrForceAction>();
/**
 * Action that addes the elastic driving force for each order parameter
 */
class PolycrystalElDrForceAction: public Action
{
public:
  PolycrystalElDrForceAction(const std::string & name, InputParameters params);

  virtual void act();

private:

  /// Number of order parameters used in the model
  unsigned int _op_num;

  /// Base name for the order parameters
  std::string _var_name_base;
};

#endif //POLYCRYSTAELDRFORCEACTION_H
