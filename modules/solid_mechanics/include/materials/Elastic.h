#ifndef ELASTIC_H
#define ELASTIC_H

#include "SolidModel.h"

class Elastic : public SolidModel
{
public:
  Elastic( const std::string & name,
           InputParameters parameters );
  virtual ~Elastic();

protected:

  virtual void initQpStatefulProperties() {}

  /// Compute the stress (sigma += deltaSigma)
  virtual void computeStress();

};

template<>
InputParameters validParams<Elastic>();


#endif
