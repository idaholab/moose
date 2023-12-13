# MOOSE papers authorship guidelines

MOOSE is a collaborative effort and not only do we always welcome contributions, but also want to
credit them appropriately and in a sustainable manner. We do this by including our contributors
in the relevant journal article(s) that should be used by all, both within the community and externally,
to cite the MOOSE framework and its modules. Please see the [citing.md] page for the current versions
of these articles. These articles are published and updated on a regular basis.

## MOOSE framework paper authorship guidelines

When citing MOOSE, whether when using it, using a derived application, or just referring to
the MOOSE project, the top-level MOOSE framework paper should be cited. It is featured in the [Citing MOOSE](citing.md#MOOSE) page.
This paper is regularly replaced with either a new paper or an "update" paper. "Update" papers
refer to a summary of changes related to a periodic update of the software from the timeframe of the previous "update" paper, and can have a different authorship list than the original
paper.

In order to best acknowledge the current contributors to the framework, we strive to keep the authorship
of the MOOSE framework paper representative. To do that, we publish a summary of the new developments in the framework
and use the list of the principal contributors since the previous paper as the list of authors for the new paper.

The contributors are listed in two categories.

- Semi-permanent contributors are included in every new MOOSE framework paper by default. These contributors
  have written a very large part of the MOOSE framework and must be credited, at least until their contributions
  are overtaken by others.

The current criteria for semi-permanent authorship are listed below:

- Top 10 contributor as measured by lines of code in the `framework/` and other utility folders or
- Top 10 contributor as measured by number of commits in the `framework/` folder


!alert note
Pull request reviewers will ensure that duplicate and extraneous code and commit history are trimmed before
merging. These guidelines may be reformulated in the future in terms of features if we observe
that contributors are focusing on gaming the metrics over code quality.


- Transient contributors are included in the next MOOSE framework paper for the period(s) in which they contributed.
  These contributors have played an important role in the development of MOOSE since the previous paper was published.


The current guidelines for transient authorship are listed below:

- 1 major framework contribution or
- 4 minor framework contributions and significant involvement in MOOSE-related activities or
- 10 fixup framework contributions and significant involvement in MOOSE-related activities

MOOSE-related activities are developments in the MOOSE modules and MOOSE applications.
Modules are deliberately excluded from the list of contributions, as developers of a module should
publish a module paper and report on new developments outside of the MOOSE framework paper.

A list of major, minor and fixup example contributions has been developed as guidelines for
classification and can be shared upon request. The classification of contributions is decided when
gathering the list of authors for the next paper.

!alert note
Contributors who do not meet the criteria for authorship in the paper will see their contributions from
the past period considered for the next paper.

!alert note
Code reviews and user support are currently not formalized in the criteria. Depending on the scale of the effort,
we may consider them at the level of minor and fixup contributions.

!alert note
The contribution guidelines may be scaled depending on the time elapsed between successive publications. The current
guidelines were developed for an 18 months time frame between two publications.

## MOOSE Modules paper authorship guidelines

Modules should strive to follow the same principles as the framework with regards to authorship inclusivity
and representativity. However, most modules are not developed by the framework team and have their own policies.
Please contact the relevant module team for more information. You may find some members of each module team in the
[CODEOWNERS file](https://github.com/idaholab/moose/blob/master/CODEOWNERS).
