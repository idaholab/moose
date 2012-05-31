/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

/**
 * Example 15: Custom Actions
 * This example runs our favorite Convection Diffusion problems but uses
 * a custom parser block to add the kernels all together with one new
 * block in our input file instead of listing everything explicitly
 */

//Moose Includes
#include "MooseInit.h"
#include "Executioner.h"
#include "Factory.h"
#include "ActionFactory.h"  // <- Actions are special (they have their own factory)

// Parser
#include "Parser.h"
#include "MooseSyntax.h"

// Example 14 Includes
#include "Convection.h"
#include "ConvectionDiffusionAction.h"

// libMesh includes
#include "perf_log.h"

PerfLog Moose::perf_log("Example 15: Custom Actions");

int main (int argc, char** argv)
{
  MooseInit init (argc, argv);

  Moose::registerObjects();

  registerKernel(Convection);

  /**
   * Registering an Action is a little different than registering the other MOOSE
   * objects.  You register your Action class with an "action_name" that can be
   * satisfied by executing the Action (running the "act" virtual method).
   */
  registerAction(ConvectionDiffusionAction, "add_kernel");
  Moose::associateSyntax();
  Parser p(Moose::syntax);

  /**
   * Now we need to tell the parser what new section name to look for and what
   * Action object to build when it finds it.  This is done directly on the parser
   * with the registerActionSyntax method.
   *
   * The section name should be the "full path" of the parsed section but should NOT
   * contain a leading slash.  Wildcard characters can be used to replace a piece of the
   * path.
   */
  Moose::syntax.registerActionSyntax("ConvectionDiffusionAction", "ConvectionDiffusion");

  // Parse commandline and return inputfile filename if appropriate
  std::string input_filename = p.parseCommandLine();

  p.parse(input_filename);
  p.execute();

  Moose::executioner->execute();

  return 0;
}
