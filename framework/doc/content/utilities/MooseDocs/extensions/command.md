# Command Extension

The command extension does not add any functionality directly; however, it provides the mechanism
for which a majority of the existing extensions are built upon. Therefore, all custom commands
follow the same basic structure and automatically have two forms and inline and block form.

## Inline Command Format

```markdown
!command subcommand key=value key2=value with spaces
                    key3=value3
Some content that is optional, but if used continues
until the first empty line.
```

## Block Command Format

```markdown
!command! subcommand key=value key2=value with space
                     key3=value3
Content that is optional, but if used
can include empty lines.

The content ends with the "end" version
of the command.
!command-end!
```
