#ifndef FLUIDPROPERTIES_H
#define FLUIDPROPERTIES_H

#include "GeneralUserObject.h"

// Forward Declarations
class FluidProperties;

template<>
InputParameters validParams<FluidProperties>();

class FluidProperties : public GeneralUserObject
{
public:
  FluidProperties(const InputParameters & parameters);
  virtual ~FluidProperties();

  virtual void execute();
  virtual void initialize();
  virtual void finalize();
};

#endif /* FLUIDPROPERTIES_H */
