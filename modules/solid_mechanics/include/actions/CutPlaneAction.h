#ifndef CUTPLANEACTION_H
#define CUTPLANEACTION_H

#include "Action.h"
#include "MooseTypes.h"
#include "libmesh/vector_value.h"

class CutPlaneAction;

template<>
InputParameters validParams<CutPlaneAction>();

class CutPlaneAction : public Action
{
public:
  CutPlaneAction(const std::string & name, InputParameters params);
  ~CutPlaneAction();

  virtual void act();
protected:

  const std::string _order;
  const std::string _family;
};

#endif //CUTPLANEACTION_H
