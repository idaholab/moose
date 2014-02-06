#ifndef JINTEGRALACTION_H
#define JINTEGRALACTION_H

#include "Action.h"
#include "MooseTypes.h"
#include "libmesh/vector_value.h"

class JIntegralAction;

template<>
InputParameters validParams<JIntegralAction>();

class JIntegralAction : public Action
{
public:
  JIntegralAction(const std::string & name, InputParameters params);
  ~JIntegralAction();

  virtual void act();
private:
  unsigned int calcNumCrackFrontNodes();

  const std::vector<BoundaryName> & _boundary_names;
  const std::string _order;
  const std::string _family;
  RealVectorValue _crack_direction;
  bool _treat_as_2d;
  unsigned int _axis_2d;
  std::vector<Real> _radius_inner;
  std::vector<Real> _radius_outer;
  bool _use_displaced_mesh;
};

#endif //JINTEGRALACTION_H
