#ifndef NSMASSWEAKSTAGNATIONBC_H
#define NSMASSWEAKSTAGNATIONBC_H

#include "NSWeakStagnationBC.h"

// Forward Declarations
class NSMassWeakStagnationBC;

template<>
InputParameters validParams<NSMassWeakStagnationBC>();


/**
 * The inviscid energy BC term with specified normal flow.
 */
class NSMassWeakStagnationBC : public NSWeakStagnationBC
{

public:
  NSMassWeakStagnationBC(const std::string & name, InputParameters parameters);

  virtual ~NSMassWeakStagnationBC(){}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);
};

#endif // NSMASSWEAKSTAGNATIONBC_H
