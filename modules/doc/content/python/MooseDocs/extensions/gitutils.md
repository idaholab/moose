# GitUtils Extension

The "gitutils" extension provides a mechanism for gleaning information regarding the git repository
that the documentation build command is being executed within.

!devel settings module=MooseDocs.extensions.gitutils
                object=GitUtilsExtension
                id=gitutils-config
                caption=Configuration items for the "gitutils" extension.

## Commit

The "commit" sub-command inserts the current commit SHA-1 for the repository that the documentation
build occurs as demonstrated in [gitutils-commit-example].
The available settings for the this command are provided in [gitutils-commit-settings].

!devel settings module=MooseDocs.extensions.gitutils
                object=CommitCommand
                id=gitutils-commit-settings
                caption=Available settings for the `!git commit` command.

!devel! example id=gitutils-commit-example
               caption=Example of reporting the current commit.
!git commit

The current commit is [!git!commit].
!devel-end!
