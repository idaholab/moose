
#include "braceexpr.h"

std::string
BraceNode::str(int indent)
{
  std::string indstr;
  for (int i = 0; i < indent; i++)
    indstr += "    ";

  if (_val != "")
    return indstr + _val + "\n";

  std::string s = indstr + "${\n";
  for (auto & n : _list)
  {
    s += n.str(indent + 1);
  }
  s += indstr + "}\n";
  return s;
}

BraceNode &
BraceNode::append()
{
  _list.push_back(BraceNode());
  return _list.back();
}

std::string
EnvEvaler::eval(std::list<std::string> & args)
{
  std::string var = args.front();
  std::string val;
  auto data = std::getenv(var.c_str());
  if (data)
    val = data;
  return val;
}

std::string
RawEvaler::eval(std::list<std::string> & args)
{
  std::string s;
  for (auto & arg : args)
    s += arg;
  return s;
}

size_t parseBraceNode(const std::string & input, size_t start, BraceNode & n);
size_t parseBraceBody(const std::string & input, size_t start, BraceNode & n);

void
BraceExpander::registerEvaler(const std::string & name, Evaler & ev)
{
  _evalers[name] = &ev;
}

std::string
BraceExpander::expand(const std::string & input)
{
  std::string result = input;
  size_t start = 0;
  while ((start = result.find("${", start)) != std::string::npos)
  {
    BraceNode root;
    parseBraceNode(result, start, root);

    // skip brace expressions that are a single word - they are a special case for intra-input
    // replacement based on HIT paths.
    if (root.list().size() < 2)
    {
      start += root.len();
      continue;
    }

    auto replace_text = expand(root);
    result.replace(root.offset(), root.len(), replace_text);
    start = root.offset() + replace_text.size();
  }
  return result;
}

std::string
BraceExpander::expand(BraceNode & expr)
{
  auto args = expr.list();
  if (args.size() == 0)
    return expr.val();

  std::list<std::string> expanded_args;
  for (auto it = args.begin(); it != args.end(); ++it)
    expanded_args.push_back(expand(*it));

  auto cmd = expanded_args.front();
  if (_evalers.count(cmd) == 0)
    throw std::runtime_error("no valid evaler '" + cmd + "'");
  expanded_args.pop_front();
  return _evalers[cmd]->eval(expanded_args);
}

size_t
skipSpace(const std::string & input, size_t start)
{
  const std::string space = "\n\t \r";
  while (start < input.size() && space.find(input[start]) != std::string::npos)
    start++;
  return start;
}

size_t
untilSpace(const std::string & input, size_t start)
{
  const std::string space = "\n\t \r}";
  while (start < input.size() && space.find(input[start]) == std::string::npos)
    start++;
  return start;
}

size_t
parseBraceNode(const std::string & input, size_t start, BraceNode & n)
{
  n.offset() = start;
  start += 2; // eat opening "${"
  start = parseBraceBody(input, start, n);
  start = skipSpace(input, start);
  if (input[start] != '}')
    throw std::runtime_error("missing closing '}' in brace expression");
  start++; // eat closing "}"
  n.len() = start - n.offset();
  return start;
}

size_t
parseBraceBody(const std::string & input, size_t start, BraceNode & n)
{
  start = skipSpace(input, start);
  while (start < input.size() && input[start] != '}')
  {
    if (input.find("${", start) == start)
      start = parseBraceNode(input, start, n.append());
    else
    {
      auto end = untilSpace(input, start);
      auto & child = n.append();
      child.val() = input.substr(start, end - start);
      start = end;
    }
    start = skipSpace(input, start);
  }
  return start;
}
