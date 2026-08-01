# FileTester

Base class for all the testers comparing output files to reference gold files.
It handles the selection of the `gold` folder, containing the reference files,
the optional `output_dir` for locating produced output files,
and the value of the absolute and relative tolerances.
