#ifndef ELK_H
#define ELK_H

class Factory;
class ActionFactory;
class Syntax;

namespace Elk
{
  void registerApps();
  void registerObjects(Factory & factory);
  void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
}

#endif //ELK_H
