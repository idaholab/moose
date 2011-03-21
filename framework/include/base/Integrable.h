#ifndef INTEGRABLE_H_
#define INTEGRABLE_H_

namespace Moose {

class Integrable
{
public:
  virtual void computeResidual() = 0;
  virtual void computeJacobian(int /*i*/, int /*j*/) { }
};

} // namespace

#endif /* INTEGRABLE_H_ */
