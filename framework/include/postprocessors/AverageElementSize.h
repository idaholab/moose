#ifndef AVERAGEELEMENTSIZE_H
#define AVERAGEELEMENTSIZE_H

#include "ElementAverageValue.h"

//Forward Declarations
class AverageElementSize;

template<>
InputParameters validParams<AverageElementSize>();

/**
 * This postprocessor computes an average element size (h) for the whole domain.
 */
class AverageElementSize : public ElementAverageValue
{
public:
  AverageElementSize(const std::string & name, InputParameters parameters);

  virtual void initialize();
  virtual void execute();

  virtual Real computeIntegral();

  virtual Real getValue();

protected:
  int _elems;
};
 
#endif // AVERAGEELEMENTSIZE_H
