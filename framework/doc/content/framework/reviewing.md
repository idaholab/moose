# Reviewing Pull Requests

MOOSE is a large project with large numbers of users and contributors.
Reviewers are bound by strict software quality standards, which they
enforce through careful reviews, often at the expense of their own research.
The guidelines outlined on this page are designed to help reviewers
provide an environment welcoming of external contributions and deal with
common situations.

!alert note title=External reviews welcome
Software quality assurance requires that pull requests be reviewed by at least one member of the Change Control Board (CCB). However, anyone, including those external to the CCB, may add their own reviews.

## Conduct

- The MOOSE project strives to create a respectful environment for all contributors, reviewers, and users. Therefore, please be respectful during the pull request submission and review process. Pull requests can be closed if interactions do not meet this standard. 
- Try to be helpful. The reviewer is not expected to implement changes
  for the author, but the reviewer should try to ensure that
  the author has some direction, particularly if the author is a new
  or infrequent contributor.
- Make it clear what changes are required, versus suggestions.
  For example, use "I suggest" for suggestions and imperative language,
  e.g., "Do this", for required changes.
  Anything minor (typos, grammar, docstrings) is assumed to be required. If you
  do not wish for something to be changed, do not suggest the change.
- If it would be helpful, provide the author with a link to the contributing
  guidelines page.
- Feel free to ask questions. It's normal to not understand everything in the pull request when you have
  not written that code. Many times, the response to these questions should actually be added as comments
  in the code.
- It is acceptable to make commits to the contributor's feature branch.
  Note that NQA-1 explicitly allows this, and an additional, independent
  review is only required if the additional commit(s) are significant.
- If the author refuses the requested changes, request an additional
  reviewer (ideally a `CODEOWNER` for the affected lines) to make a
  final decision on the changes.
- If your comments are ignored / marked as resolved with no action, engage again on those same lines.
  If this does not bring resolution, seek an additional reviewer.
- If a pull request is complex, it is ok to ask that it is split in several pull requests. It is also ok to refuse
  "rider" commits changing code unrelated to the pull request at hand.
- Be timely. Pull requests should only linger when there are good reasons for them not to move forward.
  If you cannot review, consider delegating the review to another member.
- If there is a section of the code that you are not familiar with,
  feel free to ask for another reviewer's opinion.

## Review Strategy

- Review the high-level design of the changes before reviewing finer
  details.
- Consider the scope of the changes when determining how thorough
  the review should be: changes that potentially affect a large
  number of users should be subjected to greater scrutiny. Also
  consider the likelihood of additional capability being built
  on top of these changes.
  If the proposed changes do not allow for additional capability, this can be a reason
  to refuse them.
- Carefully consider the user interface, such as input parameters,
  particularly for potentially high profile changes.
- Consider how the code will be understood outside of the context of a pull request.
  The comments and documentation should be enough to understand what is going on without
  asking the initial developers.

## Testing

- At the very least, testing should cover all the features contributed, and most of the relevant code lines.
  Note that code coverage is automatically checked to meet a minimum
  value, but the reviewer may decide to override this failure if
  deemed justified. For example, a change may involve some branches
  in conditionals that should be impossible to hit.
- The reviewer may request additional testing.
- The reviewer may enable additional testing recipes. Anything with manual memory management
  could benefit from a valgrind recipe. Anything with loose tolerances is likely to fail parallel testing.
  Feel free to add those recipes.

## Documentation

- Inline code documentation such as doxygen is not required but may be requested
  when code is unclear. When in doubt, ask for more documentation.
- Request documentation for new classes and systems and also for
  modifications to existing documentation where it is deemed important.
- Thorough documentation is not required, but it should be accurate
  and error-free, and it should clarify anything unusual or unclear.
- Ensure that existing documentation remains accurate with code changes.
- If the changes are significant enough to communicate to the
  community, ensure there is a corresponding newsletter entry.

## Grammar, Spelling, and Typos

- Inline typos and grammar and spelling mistakes are recommended
  to be fixed, but are not as high of a priority as correctness in
  external documentation.
- Use Github suggestions for grammar, spelling, and typos.
