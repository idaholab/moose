
# GitHub Backups

To fulfill auditing requirements, the [!ac](MOOSE) repository must have a backup on an internal,
[!ac](INL)-based service. This is accomplished in two ways:

1. A pull mirror is set up from the GitHub repository to [https://gitlab.software.inl.gov](https://gitlab.software.inl.gov),
   which updates the source code of all branches whenever pushes are made.
2. A cron script is run every two weeks on Friday to manually retrieve all issues and pull requests
   and copy them into an internal backup CSV file. *This is a full backup each time, not an incremental
   backup.* This is necessary because repository mirroring between GitHub and GitLab does +NOT+ retrieve
   issues and PRs, due to them being implemented slightly differently between the two services.

Backup for issues and pull requests in (2) is accomplished by:

1. querying the GitHub [!ac](API) for a JSON file of the issues,
2. using internal Python tools to parse the JSON into a dictionary, then
3. dumping the dictionary into a CSV file.

Backups are maintained for at least two years; after which, older backups may be removed at any time
to free up disk space. These backups can be located on any [!ac](INL) [!ac](HPC) machine inside of
the location:

```
/home/mooseadmin/auditingbackups/(repository name)
```

## Restoring Backups

!style halign=left
If the backup is ever needed for a restoration of [!ac](MOOSE) on GitHub, the CSV can be imported
using the +GitHub CSV Tools+ utility, available [here](https://www.npmjs.com/package/github-csv-tools).
If, for whatever reason, the backed-up issues need to be introduced to GitLab, this can be accomplished
by simply renaming the `body` header in the CSV to `description` (the script can similarly be altered
to save the CSV in the GitLab format by replacing it in there as well).

!alert! warning title=The built-in GitLab issue importer is not recommended!
We +strongly+ recommend against using the built-in GitLab issue importer, as it will not be able to
import anything from the GitHub version of the repository except for the title and description of the
issue. Using the GitLab importer will also result in the loss of every pull request, as GitHub considers
all pull requests to be issues, but GitLab views them as a separate data structure.
!alert-end!
