#ifndef NSMOMENTUMWEAKSTAGNATIONBC_H
#define NSMOMENTUMWEAKSTAGNATIONBC_H

#include "NSWeakStagnationBC.h"

// Forward Declarations
class NSMomentumWeakStagnationBC;

template<>
InputParameters validParams<NSMomentumWeakStagnationBC>();


/**
 * The inviscid energy BC term with specified normal flow.
 */
class NSMomentumWeakStagnationBC : public NSWeakStagnationBC
{

public:
  NSMomentumWeakStagnationBC(const std::string & name, InputParameters parameters);

  virtual ~NSMomentumWeakStagnationBC(){}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Required parameters
  unsigned _component;
};

#endif // NSMOMENTUMWEAKSTAGNATIONBC_H
