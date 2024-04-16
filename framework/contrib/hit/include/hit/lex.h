#pragma once

#include <string>
#include <vector>
#include <memory>

namespace hit
{

// the EOF macro wreaks havok on our TokType enum which has an EOF member. So undefine it and the
// redefine it at the end of the file.
#define TMPEOF EOF
#undef EOF

// TokType
enum class TokType
{
  Error,
  EOF,
  Equals,
  LeftBracket,
  RightBracket,
  Ident,
  Path,
  Number,
  String,
  Comment,
  InlineComment,
  BlankLine,
};

/// Token represents an (atomic) token/quantum of input text.
struct Token
{
  Token(TokType t,
        const std::string & val,
        const std::string & name,
        size_t offset = 0,
        int line = 0,
        int column = 0);
  /// str returns a human-friendly string representation of the token.
  std::string str() const;

  /// type identifies the category/type of the token (i.e. String, Number, Comment, etc.)
  TokType type;
  /// val is the actual text from the input that makes this token.
  std::string val;
  /// name of the original input file
  std::string name;
  /// offset is the byte offset into the original input identifying the start position where this
  /// token was found.  This can be used to determine line numbers, column offsets, etc. useful for
  /// error messages among other things.
  size_t offset;
  /// line is the line number of the original input (lines deliminted by the unix newline '\n'
  /// character) on which the beginning of the token was found - this is redundant with the offset
  /// combined with a reference to the original input, but is here for convenience.
  int line;
  /// column is the char position after the preceeding \n (or beginning of the file) at which
  /// the beginning of the token was found - this is redundant with the offset
  /// combined with a reference to the original input, but is here for convenience.
  int column;
};

class Lexer;

/// This is a screwy hack to allow a typedef for a function that returns the type of the typedef
/// (i.e. recursive typedef).  The return type of these functions must be _LexFunc, while the
/// type(def) of these functions is LexFunc.  To use the lexer, you need to create a set of
/// LexFunc's that use a Lexer instance to consume the input emitting tokens.  Each LexFunc returns
/// the next LexFunc to be executed or nullptr if either the input is finished or there were any
/// lexing errors.
struct _LexFunc;
typedef _LexFunc (*LexFunc)(Lexer *);
struct _LexFunc
{
  _LexFunc(LexFunc pp);
  operator LexFunc();
  LexFunc p;
};

/// charIn is a convenience function for writing LexFunc's that returns true if the given character
/// c is from the given valid set.  It returns false otherwise.
bool charIn(char c, const std::string & valid);

/// lexHit is the starting LexFunc implementation for tokenizing a hit input text - i.e. pass
/// this to Lexer::run to tokenize a hit input.
_LexFunc lexHit(Lexer * l);

/// Lexer is the workhorse that manages lexographical traversal of the input text.  It keeps track
/// of the current position w.r.t. consumption of the input and provides many convenience functions
/// for moving through the input.
class Lexer
{
public:
  Lexer(const std::string & name, const std::string & input);
  /// tokens returns the list of tokens generated from the lexing process that represent the input
  /// text provided to this lexer.  tokens will be empty until after the run function has been
  /// called and completes execution.
  std::vector<Token> & tokens();
  /// run executes the lexer causing it to processes the input and generate a sequence of tokens
  /// representing it.  start is the first, starting LexFunc.  run calls start, passing in this
  /// lexer instance as an argument.  If start returns another LexFunc, it is called with this lexer
  /// instance as an argument, and so forth until the current LexFunc returns nullptr - at which
  /// point lexing terminates.
  std::vector<Token> run(LexFunc start);

  /// emit finalizes and creates+adds a token of the given type to the list of tokens representing
  /// the lexer's input text (i.e. the tokens returned by the tokens function).  The
  /// created/emitted token is automatically populated with its value set to the string delineated by
  /// the lexer's start and pos offsets into the input text.  emit then advanes the lexer start
  /// offset to the current pos.
  void emit(TokType type);

  /// lastToken returns the offset of the last character of the most recent emitted token.
  size_t lastToken();

  /// error emits an error token with the given messsage.  For convenience returns an nullptr
  /// LexFunc that can be directly returns by any LexFunc that calls error.
  LexFunc error(const std::string & msg);

  /// next consumes and returns the next byte of input, advancing the lexer's position
  /// offset by one.
  char next();

  /// rewind resets the current position and start offsets backward to the first character
  /// following the last emitted token.
  void rewind();

  /// accept conditionally accepts the next byte of input if it is one of the characters in the
  /// valid set.  It returns true if a character was consumed and false otherwise.
  bool accept(const std::string & valid);

  /// acceptRun accepts a run of zero or more consecutive characters (greedily) that must be from
  /// the given valid set.  It returns the number of characters consumed.
  int acceptRun(const std::string & valid);

  /// peek returns the next character of input without advancing the lexer's position offset (i.e.
  /// without consuming the character).
  char peek();

  /// ignore advances the lexer's start offset to the current position offset without emitting any
  /// tokens - effectively skipping a portion of the input text.
  void ignore();
  /// backup unconsumes the most recently consumed byte of input, reducing the position offset by
  /// one.  This should usually only be called once after each call to next.
  void backup();

  /// input returns the full input text the lexer is operating on i.e. the entire input string it
  /// was initialized/constructed with.
  const std::string & input();
  /// start returns the current start byte offset into the input identifying the start of the next
  /// token that will be emitted (or next section of input that will be ignored).
  size_t start();
  /// pos returns the byte offset of the lexer's current position in the input text - i.e. the
  /// location up to which input has been consumed/seen.
  size_t pos();

private:
  int _line_count = 1;
  std::string _name;
  std::string _input;
  size_t _start = 0;
  size_t _pos = 0;
  int _width = 0;
  std::vector<Token> _tokens;
};

/// This is *the* function in the hit namespace. It takes a given hit input text and returns a list
/// of tokens. This is used for syntax highlighting and autocomplete in the peacock editor
std::vector<Token> tokenize(const std::string & fname, const std::string & input);

#define EOF TMPEOF

} // namespace hit
