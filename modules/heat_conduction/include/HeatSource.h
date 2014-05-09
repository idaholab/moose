#ifndef HEATSOURCE_H
#define HEATSOURCE_H

#include "BodyForce.h"

//Forward Declarations
class HeatSource;

template<>
InputParameters validParams<HeatSource>();

class HeatSource : public BodyForce
{
public:
  HeatSource(const std::string & name, InputParameters parameters);
};

#endif
