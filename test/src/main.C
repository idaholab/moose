#include "Init.h"
#include "Parser.h"
#include "Executioner.h"
#include "MooseTest.h"

int
main(int argc, char *argv[])
{
  Moose::Init init(argc, argv);
  MooseTest::registerObjects();

  Parser p;

  std::string input_filename = "";
  if (Moose::command_line->search("-i"))
    input_filename = Moose::command_line->next(input_filename);
  else
    p.printUsage();

  p.parse(input_filename);
  p.execute();

  Executioner *e = p.getExecutioner();
  e->execute();
  delete e;

  return 0;
}
