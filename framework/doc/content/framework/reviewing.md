# Reviewing Pull Requests

MOOSE is a large project with large numbers of users and contributors.
While MOOSE seeks the highest standards of software quality, it
is important to recognize that requiring perfection in proposed
code changes is not realistic, given the large amount of activity,
versus the limited availability of reviewers and time.
The guidelines outlined on this page are designed to guide
reviewers in finding the right balance between efficiency
and software quality.

## Conduct

- Be respectful.
- Try to be helpful. The reviewer is not expected to implement changes
  for the author, but the reviewer should try to ensure that
  the author has some direction, particularly if the author is a new
  or infrequent contributor.
- Make it clear what are changes that you require, versus suggestions.
  For example, use "I suggest" for suggestions and imperative language,
  e.g., "Do this", for required changes.
- If it would be helpful, provide the author with a link to the contributing
  guidelines page.
- It is acceptable to make commits to the contributor's feature branch.
  Note that NQA-1 explicitly allows this, and an additional, independent
  review is only required if the additional commit(s) are significant.
- If the author refuses the requested changes, request an additional
  reviewer (ideally a `CODEOWNER` for the affected lines) to make a
  final decision on the changes.

## Review Strategy

- Review the high-level design of the changes before reviewing finer
  details.
- Consider the scope of the changes when determining how thorough
  the review should be: changes that potentially affect a large
  number of users should be subjected to greater scrutiny. Also
  consider the likelihood of additional capability being built
  on top of these changes.
- Carefully consider the user interface, such as input parameters,
  particularly for potentially high profile changes.
- Prioritize discovery of bugs that would manifest in very subtle,
  difficult-to-discover ways, rather than those bugs that
  would immediately become apparent, such as by an immediate error message.

## Testing

- The reviewer is not expected to test out the proposed
  changes, but this may be recommended in some cases when the
  regression testing is deemed insufficient.
- At the very least, testing should cover most of the new code lines.
  Note that code coverage is automatically checked to meet a minimum
  value, but the reviewer may decide to override this failure if
  deemed justified. For example, a change may involve some branches
  in conditionals that should be impossible to hit.
- The reviewer may request additional testing.

## Documentation

- Inline code documentation such as doxygen is not required but may be requested
  when code is unclear.
- Request documentation for new classes and systems and also for
  modifications to existing documentation where it is deemed important.
- Thorough documentation is not required, but it should be accurate
  and error-free, and it should clarify anything unusual or unclear.
- If the changes are significant enough to communicate to the
  community, ensure there is a corresponding newsletter entry.

## Grammar, Spelling, and Typos

- Inline typos and grammar and spelling mistakes are recommended
  to be fixed, but are not as high of a priority as correctness in
  external documentation.
- Use Github suggestions for grammar, spelling, and typos.
