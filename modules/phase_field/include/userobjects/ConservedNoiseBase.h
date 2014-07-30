#ifndef CONSERVEDNOISE_H
#define CONSERVEDNOISE_H

#include "ElementUserObject.h"

//Forward Declarations
class ConservedNoiseBase;

template<>
InputParameters validParams<ConservedNoiseBase>();

/**
  * This Userobject is the base class of Userobjects that generate one
  * random number per timestep and quadrature point in a way that the integral
  * over all random numbers is zero. This can be used for a concentration fluctuation
  * kernel such as ConservedLangevinNoise, that keeps the total concenration constant.
  *
  * \see ConservedUniformNoise
  */
class ConservedNoiseBase : public ElementUserObject
{
public:
  ConservedNoiseBase(const std::string & name, InputParameters parameters);

  virtual ~ConservedNoiseBase();

  virtual void initialize();
  virtual void execute();
  virtual void threadJoin(const UserObject & y);
  virtual void finalize();

  Real getQpValue(dof_id_type element_id, unsigned int qp) const;

protected:
  virtual Real getQpRandom() = 0;

  LIBMESH_BEST_UNORDERED_MAP<dof_id_type, std::vector<Real> > _random_data;

  Real _integral;
  Real _volume;
  Real _offset;

  unsigned int _qp;
};

#endif //CONSERVEDNOISE_H
