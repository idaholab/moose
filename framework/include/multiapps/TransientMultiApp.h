#ifndef TRANSIENTMULTIAPP_H
#define TRANSIENTMULTIAPP_H

#include "MultiApp.h"
#include "MooseApp.h"
#include "Transient.h"
#include "TransientInterface.h"

class TransientMultiApp;

template<>
InputParameters validParams<TransientMultiApp>();

/**
 * MultiApp Implementation for Transient Apps.
 * In particular, this is important because TransientMultiApps
 * will be taken into account in the time step selection process.
 */
class TransientMultiApp :
  public MultiApp,
  public TransientInterface
{
public:
  TransientMultiApp(const std::string & name, InputParameters parameters);

  ~TransientMultiApp();

  /**
   * Advance all of the apps one timestep.
   */
  void solveStep();

  /**
   * Finds the smallest dt from among any of the apps.
   */
  Real computeDT();

private:
  std::vector<Transient *> _transient_executioners;
};

#endif // TRANSIENTMULTIAPP_H
