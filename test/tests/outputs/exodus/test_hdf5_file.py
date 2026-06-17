#!/usr/bin/env python3
import argparse
import sys
import h5py


def is_hdf5(filepath: str) -> bool:
    try:
        with h5py.File(filepath, "r"):
            return True
    except OSError:
        return False


def main():
    parser = argparse.ArgumentParser(
        description="Check whether a file is in HDF5 format."
    )
    parser.add_argument(
        "filepath",
        help="Path to the file to check.",
    )
    parser.add_argument(
        "expected",
        type=lambda x: x.lower() in ("true", "1", "yes"),
        metavar="IS_HDF5",
        help="Expected result: true if you expect the file to be HDF5, false otherwise.",
    )
    args = parser.parse_args()

    result = is_hdf5(args.filepath)
    status = "HDF5" if result else "not HDF5"
    print(f"{args.filepath} is {status}.")

    if result == args.expected:
        print("Result matches expectation.")
        sys.exit(0)
    else:
        print(f"Result does NOT match expectation (expected: {args.expected}).")
        sys.exit(1)


if __name__ == "__main__":
    main()
