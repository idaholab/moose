# Code Review

## Review Guidelines

- Reviews should be thoughtful
- Reviews should be thorough; we don't want bugs! But
  - Don't let perfect be the enemy of the good
  - We'd like development to be agile
  - Merged code will have eyes of tens of MOOSE contributors; runs of hundreds
    to thousands of users
  - Unmerged code will have eyes of much fewer contributors and runs of much
    fewer users
    - No code, no bug reports! But also no features
  - People will be hesitant to contribute if the review process is onerous
- Are there things we can do to abate fear of bugs?
  - Request more tests
- As a reviewer, try to make it clear to the author what conversations are
  required changes vs. suggestions (perhaps GitHub Requested Change vs Comment?)
- NQA-1 explicitly allows for a change-control board member to push commits to a
  pull request and maintain independent reviewer status as long as the pushed
  commits are not "significant."
- Reviews should be prompt. We should strive to provide a first quick review within XX business days
- New features should come under more scrutiny than re-factoring, which benefits from the confidence offered by the existing test suite

## Addressing Deadlock Between a CCB Author and CCB Reviewer

Procedure

- Mention a second (or more) CCB reviewer on the disputed conversations. Hopefully
  additional reviewer(s) can help massage the conversation into resolution
- If no mediation, the additional CCB reviewer breaks the tie
- CCB member chosen for mediation/tie-breaking ideally is a CODEOWNER over the
  disputed lines
