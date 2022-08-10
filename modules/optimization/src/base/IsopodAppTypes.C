#include "IsopodAppTypes.h"
#include "ExecFlagRegistry.h"

namespace IsopodAppTypes
{
const ExecFlagType EXEC_FORWARD = registerExecFlag("FORWARD");
const ExecFlagType EXEC_ADJOINT = registerExecFlag("ADJOINT");
const ExecFlagType EXEC_HOMOGENEOUS_FORWARD = registerExecFlag("HOMOGENEOUS_FORWARD");
}
