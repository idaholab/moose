# Adding New V&V Cases

Verification and validation documentation is located in `doc/content/v_and_v`.
It has the following structure:

| Directory/File name | Description |
| :- | :- |
| `verification/` | Directory with verification cases |
| `validation/` | Directory with validation cases |
| `index.md` | Index with all V&V cases |

## Adding a New Case

To add a new case, create a new `.md` file either in `verification` or `validation` directory.
Ideally, you should be using the provided template to fill in information about the V&V case, as shown below:

```
!template load file=validation_case.md.template
  name=The Name Of The V&V Test

!template! item key=introduction
Describe why this test case was selected and how it contributes to the V&V.
!template-end!

!template! item key=problem-description
Describe the test apparatus, provide pictures, etc.
!template-end!

!template! item key=model-description
Describe the model used to simulate the experiment.
!template-end!

!template! item key=tests-simulated
List the test performed and include the test results
!template-end!
```

## Citing Other Documents

If you want to reference other documents, create a `.bib` file with the same base name as your `.md` file above.
Add BibTeX references into this file.
Then, use the citation markdown to cite the BibTeX entries as follows

| Markdown | Result |
| :- | :- |
| `[!cite](gaston2015physics)` | [!cite](gaston2015physics) |
| `[!citep](gaston2015physics)` | [!citep](gaston2015physics) |

You can list multiple entries in a citation tag like this: `[!cite](label1, label2, label3)`.

## Updating the List of V&V Cases

Open `index.md` and add a new row into either the verification or validation table.
You will need to supply a name and a short description of the case.
