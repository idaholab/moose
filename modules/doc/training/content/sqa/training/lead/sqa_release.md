# Performing MOOSE and MOOSE-based Application Release(s)

!---

## PLN-4005 and Releases

!alert warning title=Refer to PLN-4005
PLN-4005 details what must be performed during a release of MOOSE and MOOSE-based applications, and
it should be utilized as a reference. This presentation is an introduction to aid in following the
required process that is documented in PLN-4005

!---

## Overview of Release Process

1. Select a stable (main/master branch) for release.
1. Download testing results.
1. Complete second review.
1. Document the review by creating "release log."
1. Update MooseDocs config.yml to point to downloaded results.
1. Commit the results and log.
1. Use `git tag` to label the release.
1. Push the tag to the main repository.

!---

## Select Revision for Release

Any revision of MOOSE or a MOOSE-based application can be selected for a release as long as all
the testing is complete and passed.

!alert note title=Releases +must+ depend on releases
A MOOSE-based application release must use a released version of MOOSE and dependent
applications. When releasing an application a Merge/Pull Request should be created with the
MOOSE and application submodules updated to the release tags. The following process is then followed
after merging and testing of this change is complete.

!---

## Download Results

Checkout the branch for release and execute the `git log` command.

```
$ git log
commit b657ae26aae14fbc91ad2651a364cfad80c2b9ac (HEAD -> master, upstream/master)
Merge: b107961dbd 4812111857
Author: moosetest <bounces@inl.gov>
Date:   Wed Sep 29 11:22:23 2021 -0600

    Merge commit '4812111857205877df7bb943958f9cd1f32b0d27'
```

Use these hashes to download results from CIVET (see links below).
The tag.gz files should be placed in the "release" directory.

!style style=font-size:60%;
[`https://civet.inl.gov/sha_events/idaholab/moose/b657ae26aae14fbc91ad2651a364cfad80c2b9ac/`](https://civet.inl.gov/sha_events/idaholab/moose/b657ae26aae14fbc91ad2651a364cfad80c2b9ac/)\\
[`https://civet.inl.gov/sha_events/idaholab/moose/4812111857205877df7bb943958f9cd1f32b0d27/`](https://civet.inl.gov/sha_events/idaholab/moose/4812111857205877df7bb943958f9cd1f32b0d27/)

!---

## Perform Release Review

PLN-4005 details the necessary steps for performing the release which includes performing a final
review of MOOSE or a MOOSE-based application. This review +must+ be performed by the Project Lead and
is intended to ensure that all requirements are complete, accurate, and are satisifed and that all
[!ac](SQA) documentation is correct and complete.

!---

## Create Release Log

The "release log" provides evidence that the final review of software was conducted. This log is
a text file ("RELEASE_LOG.md") that must be added to the "release" directory of the repository.

!alert note
Please refer to the current version of PLN-4005 to determine what is necessary. Do not
rely on a log from a previous release. The necessary components of the log might have been
altered since the previous release.

!---

## Update MooseDocs config.yml

To ensure that the results displayed on the generated website are correct and pulled from the
stored results, the "config.yml" must be updated to disable downloading and enable using the
results in the "release" directory. The following are the changes to the "config.yml" for a
release of "blackbear".

```
$ git diff
diff --git a/doc/config.yml b/doc/config.yml
index b46b6c19..a97a9828 100644
--- a/doc/config.yml
+++ b/doc/config.yml
@@ -46,17 +46,21 @@ Extensions:
         active: True
     MooseDocs.extensions.civet:
-        test_results_cache: '/tmp/civet/jobs'
         remotes:
             blackbear:
+                download_test_results: false
+                test_results_cache: ../release
                 url: https://civet.inl.gov
                 repo: idaholab/blackbear
                 location: ${ROOT_DIR}
             moose:
+                download_test_results: false
+                test_results_cache: ../moose/release
                 url: https://civet.inl.gov
                 repo: idaholab/moose
                 location: ${MOOSE_DIR}
```

!---

## Commit Release

After the testing results are downloaded, the release log created, and the "config.yml" is
created, the additions must be committed to the repository. It is suggested that the following
commit message pattern be followed.

```
git commit release -m "2021-09-15 release"
```

!---

## Tag and Push Release

The release is completed by tagging it with git and pushing the tag to the main remote repository.
The tag must follow the YYYY-MM-DD pattern. The following is an example of creating the tag
and pushing the change to the main remote repository (upstream).

```
git tag 2021-09-15
git push upstream
```

!alert note
Depending on your configuration the main repository may be named something other than "upstream."

!---

## Upload Release Website (optional)

Although not required by PLN-4005 the website for the release should be uploaded for consumption
by users of MOOSE or the MOOSE-based application. Please contact the MOOSE developers for assistance
with this process.
