#ifndef NSMOMENTUMPRESSUREWEAKSTAGNATIONBC_H
#define NSMOMENTUMPRESSUREWEAKSTAGNATIONBC_H

#include "NSWeakStagnationBC.h"

// Forward Declarations
class NSMomentumPressureWeakStagnationBC;

template<>
InputParameters validParams<NSMomentumPressureWeakStagnationBC>();


/**
 * This class has been deprecated.  Use one or both of the
 * .) NSMomentumConvectiveWeakStagnationBC
 * .) NSMomentumPressureWeakStagnationBC
 * classes instead.
 */
class NSMomentumPressureWeakStagnationBC : public NSWeakStagnationBC
{

public:
  NSMomentumPressureWeakStagnationBC(const std::string & name, InputParameters parameters);

  virtual ~NSMomentumPressureWeakStagnationBC(){}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Required parameters
  unsigned _component;
};

#endif // NSMOMENTUMPRESSUREWEAKSTAGNATIONBC_H
