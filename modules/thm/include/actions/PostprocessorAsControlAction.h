#ifndef POSTPROCESSORASCONTROLACTION_H
#define POSTPROCESSORASCONTROLACTION_H

#include "MooseObjectAction.h"

class PostprocessorAsControlAction;

template <>
InputParameters validParams<PostprocessorAsControlAction>();

/**
 * This action creates a control value named the same as the postprocessor being added
 *
 * This allows people to use the postprocessor values directly within the control system.
 */
class PostprocessorAsControlAction : public MooseObjectAction
{
public:
  PostprocessorAsControlAction(InputParameters params);

  virtual void act();
};

#endif /* POSTPROCESSORASCONTROLACTION_H */
