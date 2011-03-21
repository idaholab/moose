#ifndef ELASTIC_H
#define ELASTIC_H

#include "MaterialModel.h"

// Forward declarations
class ElasticityTensor;
class Elastic;

template<>
InputParameters validParams<Elastic>();

class Elastic : public MaterialModel
{
public:
  Elastic( const std::string & name,
           InputParameters parameters );
  virtual ~Elastic();

protected:

  /// Compute the stress (sigma += deltaSigma)
  virtual void computeStress();

};

#endif
