/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "Moose.h"

/// Exec flag used to execute MooseObjects while elements are being
/// marked for cutting by XFEM
extern const ExecFlagType EXEC_XFEM_MARK;
