#ifndef INSEXPLICITEXECUTIONER_H
#define INSEXPLICITEXECUTIONER_H

#include "Transient.h"

class INSExplicitExecutioner;

template<>
InputParameters validParams<INSExplicitExecutioner>();

/**
 *
 */
class INSExplicitExecutioner : public Transient
{
public:
  INSExplicitExecutioner(const std::string & name, InputParameters parameters);
  virtual ~INSExplicitExecutioner();

protected:
  virtual Real computeDT();

  /// Value of dt computed by a post processor
  PostprocessorValue & _dt_pps;
};


#endif /* INSEXPLICITEXECUTIONER_H */
