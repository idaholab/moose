# NQA-1

## Quality Assurance Requirements for Nuclear Facility Applications

!---

# NQA-1

A regulatory standard created by the [!ac](ASME) governing the
supply of items or services, which provide a safety function for nuclear facilities. +Software is
considered an "item"+.

INL's contract with DOE requires us to abide by NQA-1 standards where applicable.

!---

# NQA-1: SQA Program at INL

The [!ac](SQA) program at [!ac](INL) implements the
contractual requirements and [!ac](DOE) expectations of 10 CFR 830 Subpart A,
"Quality Assurance Requirements;" DOE Order 414.1D, "Quality Assurance;" and the American Society
of Mechanical Engineers (ASME) NQA-1-2008/1a-2009 and 2017 editions, "Quality Assurance
Requirements for Nuclear Facility Applications," which apply to software.

These requirements are
implemented in the following [!ac](INL) documents: PDD-13610, "Software Quality Assurance Program;" and
LWP-13620, "Managing Information Technology Assets," and the associated templates and forms.

!---

# NQA-1: SQA Program at INL

SQA is a set of activities whereby software engineering and quality processes and methods are
followed to ensure proper quality is achieved. A graded approach is applied commensurate with the
consequence of failure of the software:

- more rigor is applied with respect to controls and other work
  activities if the software meets the DOE definition of safety software and has a high consequence of
  failure; and
- less rigor is applied if the software is non-safety software and has a lower consequence
  of failure.

!---

# NQA-1: SQA and MOOSE

- MOOSE and many MOOSE-based applications are moving beyond being "research" projects.
- These tools +must+ be developed following the NQA-1 process as defined by
  the [!ac](DOE) and [!ac](INL).

!---

# NQA-1: Process

- The NQA-1 process is designed around traditional "waterfall" software development model.

!style halign=center
!media training/waterfall.png

!---

# NQA-1: Standard

- The NQA-1 is a standard to be satisfied, how it is satisfied is not defined.
- It is possible to use modern software development processes and satisfy the standard.

!style halign=center
!media training/agile2.png style=width:50%;

!---

# NQA-1: Traceability

The connection (traceability) between four items is the key to satisfying the NQA-1 standard.

1. +Change Request (CR)+: Why is the code being changed?
1. +Requirement+: What is the required function of the code?
1. +Design+: What is the design of the code for the change?
1. +Test+: How is the requirement shown to be satisfied?

!---

# NQA-1: Requirement

- +Unitary (Cohesive)+: addresses one item\\
- +Complete+: fully stated in one place with no missing information\\
- +Consistent+: does not contradict another and is consistent with all documentation\\
- +Non-Conjugated (Atomic)+: must be atomic, i.e., it does not contain conjunctions\\
- +Traceable+: meets a business need\\
- +Current+: has not been made obsolete by the passage of time\\
- +Unambiguous+: concisely stated without recourse to technical jargon\\
- +Verifiable+: implementation can be determined through inspection, test, or analysis

!style halign=center
*Refer to ["What is a Requirement?"](sqa/what_is_a_requirement.md optional=true) for more information.*
