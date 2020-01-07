# What is a Requirement?

The "Software Requirement Specification (SRS)" is created from a set of "requirements" that
must be: correct, complete, consistent, unambiquous, ranked, verifiable,
modifiable, and traceable [!citep](ieee1998recommended).

Within MOOSE the decision was made that each test is a requirement. This relationship is not
dictated by any standard, it was a decision that was made to allow for the above characteristics
to be satisfied in a maintainable manner. The following sections detail each of these
characteristics with respect to implementation within MOOSE.

!alert! note title=Writing a Requirement prefix=false
Practically, when creating requirement text only one of the aforementioned characteristics really
must be considered: +unambiguous+. The other characteristics are generally satisfied
automatically, by design, by the nature of the testing and documentation system implementation.

When writing a requirement, simply ask the question: "+Is the requirement unambiguous?+" Each
test was created for a specific reason, the requirement text should reflect this fact. Use this
as a means to communicate what the test is testing, i.e., what is the software required to
perform to satisfy the test.

Note that the requirement should be fairly concise, generally no longer than a sentence. If the
requirement is longer than a sentence, consider transferring some of the content into the
accompanying `design` markdown file.
!alert-end!


## Correct

> An SRS is correct if, and only if, every requirement stated therein is one that the software
> shall meet [!citep](ieee1998recommended).

The definition of the SRS being "correct" necessitates that each requirement be satisfied, a
one-to-one relationship between a test and a requirement makes verify the correctness a
trivial operation that may be automated. Within MOOSE all tests must pass, thus all requirements
are met, for the code to be merged into the stable (master) branch.

## Complete

> An SRS is complete if all requirements are acknowledged and treated, responses of the software
> to all realizable classes of input (both invalid and valid) exist, and references to all figures,
> tables, and diagrams in the SRS are valid [!citep](ieee1998recommended).

Each requirement is clearly defined within the test specification allowing for the list of
requirements to be automatically generated, therefore all requirements are guaranteed to be
acknowledged.

Tests are created specifically to quantify and verify the response of the software to inputs, both
valid and invalid. The word "realizable" is important as it is not practical to test all inputs
since the number of inputs is infinite. Any shortcoming to this end indicates a lack of sufficient
testing, when the testing is complete the SRS is complete.

The MooseDocs system itself guarantees that all references to tables, figures, and diagrams are
valid, if they are not testing fails until these references are defined.

## Consistent

> If an SRS does not agree with some higher-level document, it is not correct
> [!citep](ieee1998recommended).

The SRS and all associated documents that reference a requirement within MOOSE are automatically
generated within the MooseDocs system. The system maintains the interconnection of these documents,
if they are not in agreement an error is generated prior to the document being merged and released
into the stable (master) branch.

## Unambiguous

> An SRS is unambiguous if every requirement stated therein has only one interpretation
> [!citep](ieee1998recommended).

This characteristic, along with being "verifiable", is the driving factor for creating a
one-to-one connection between a test and a requirement. Each test is created for a specific reason
and tests very specific features within the software; the associated requirement then becomes
unambiguous by nature of a well-designed test. The requirement also serves as documentation for
the test, which is useful in general outside of the SRS.

## Ranked

> An SRS is ranked for importance if each requirement in it has an identifier to indicate the
> importance of stability of that particular requirement [!citep](ieee1998recommended).

Within MOOSE tests/requirements are not ranked for importance. As stated above an SRS must be
"correct", which necessitates that all requirements are met. Therefore, ranking is irrelevant.

## Verifiable

> An SRS is verifiable if there exists some process with which a person or a machine can check that
> the software product meets the requirement [!citep](ieee1998recommended).

This characteristic of an SRS is a driving factor behind the one-to-one relationship between
a requirement and a test. With such a connection the SRS is verifiable by a machine simply by
having all the tests passing.

## Modifiable

> An SRS is modifiable if changes to the requirements can be made easily, completely, and
> consistently while retaining the structure and style [!citep](ieee1998recommended).

Embedding the requirements within the test specifications treats the text for the requirement as
code. Coupled with the automatic creating of documents using MooseDocs all changes to requirements
are automatically retain proper styling; and, changing them is natural and easy for developers
because the changes are made along with traditional source code additions, deletions, and
alterations.

## Traceable

> An SRS is traceable if the origin of its requirements is clear and if it facilitates the
> referencing of each requirement in future development or enhancement documentation
> [!citep](ieee1998recommended).

Each requirement is embedded in the code repository, which for the case of MOOSE, is a git
repository. Then by its very nature of being treated as code the complete history each requirement
is tracked at all times.



!bibtex bibliography
