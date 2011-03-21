#ifndef EXPLICITSYSTEM_H_
#define EXPLICITSYSTEM_H_

#include "System.h"

// libMesh include
#include "explicit_system.h"
#include "transient_system.h"

namespace Moose {

class ExplicitSystem : public SystemTempl<TransientExplicitSystem>
{
public:
  ExplicitSystem(SubProblem & problem, const std::string & name);

protected:
};

}

#endif /* EXPLICITSYSTEM_H_ */
