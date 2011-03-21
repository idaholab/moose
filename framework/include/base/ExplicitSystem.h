#ifndef EXPLICITSYSTEM_H_
#define EXPLICITSYSTEM_H_

#include "SubProblem.h"

// libMesh include
#include "explicit_system.h"
#include "transient_system.h"

namespace Moose {

class ExplicitSystem : public SubProblemTempl<TransientExplicitSystem>
{
public:
  ExplicitSystem(Problem & problem, const std::string & name);

protected:
};

}

#endif /* EXPLICITSYSTEM_H_ */
