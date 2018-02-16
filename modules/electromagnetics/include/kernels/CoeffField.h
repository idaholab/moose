#ifndef COEFFFIELD_H
#define COEFFFIELD_H

#include "Reaction.h"

// Forward declarations
class CoeffField;

template <> InputParameters validParams<CoeffField>();

class CoeffField : public Reaction {
public:
  CoeffField(const InputParameters &parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

private:
  Real _coefficient;

  Function & _func;
};

#endif /* COEFFFIELD_H */
