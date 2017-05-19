#ifndef TERMINATE_H
#define TERMINATE_H

#include "RELAP7Control.h"

class TerminateControl;

template <>
InputParameters validParams<TerminateControl>();

/**
 * This control block will terminate a run if its input indicates so.
 */
class TerminateControl : public RELAP7Control
{
public:
  TerminateControl(const InputParameters & parameters);

  virtual void execute();

protected:
  /// The control data that indicates if the simulation should be terminated
  const bool & _terminate;
};

#endif // TERMINATE_H
