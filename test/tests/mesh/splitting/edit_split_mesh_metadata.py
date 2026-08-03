# * This file is part of the MOOSE framework
# * https://mooseframework.inl.gov
# *
# * All rights reserved, see COPYRIGHT for full restrictions
# * https://github.com/idaholab/moose/blob/master/COPYRIGHT
# *
# * Licensed under LGPL 2.1, please see LICENSE for details
# * https://www.gnu.org/licenses/lgpl-2.1.html

import argparse
import shutil
import struct
from pathlib import Path

FINGERPRINT_NAME = "SYSTEM/_moose/split_mesh_input_fingerprint"


def metadata_dir(split_dir):
    return Path(split_dir) / "meta_data_mesh.rd"


def read_string(data, offset):
    size = struct.unpack_from("@I", data, offset)[0]
    offset += struct.calcsize("@I")
    value = data[offset : offset + size].decode()
    offset += size
    return value, offset


def write_string(value):
    data = value.encode()
    return struct.pack("@I", len(data)) + data


def drop_fingerprint(split_dir):
    directory = metadata_dir(split_dir)
    header_path = directory / "header"
    data_path = directory / "data"
    header = header_path.read_bytes()
    data = data_path.read_bytes()

    offset = 0
    prefix = header[offset : offset + 2]
    offset += 2
    file_version = header[offset : offset + struct.calcsize("@I")]
    offset += struct.calcsize("@I")
    compare_hash_code = header[offset : offset + struct.calcsize("@N")]
    offset += struct.calcsize("@N")
    n_procs = header[offset : offset + struct.calcsize("@I")]
    offset += struct.calcsize("@I")
    num_data = struct.unpack_from("@N", header, offset)[0]
    offset += struct.calcsize("@N")

    n_data = []
    for _ in range(num_data):
        value = struct.unpack_from("@N", header, offset)[0]
        offset += struct.calcsize("@N")
        n_data.append(value)

    entries = []
    data_offset = 0
    for tid, count in enumerate(n_data):
        for _ in range(count):
            name, offset = read_string(header, offset)
            data_size = struct.unpack_from("@N", header, offset)[0]
            offset += struct.calcsize("@N")
            type_hash_code = header[offset : offset + struct.calcsize("@N")]
            offset += struct.calcsize("@N")
            type_name, offset = read_string(header, offset)
            has_context = header[offset : offset + struct.calcsize("@?")]
            offset += struct.calcsize("@?")

            value_data = data[data_offset : data_offset + data_size]
            data_offset += data_size
            if name != FINGERPRINT_NAME:
                entries.append(
                    (tid, name, value_data, type_hash_code, type_name, has_context)
                )

    if len(entries) == sum(n_data):
        raise SystemExit(f"Did not find {FINGERPRINT_NAME} in {directory}")

    new_n_data = [0] * num_data
    for tid, *_ in entries:
        new_n_data[tid] += 1

    new_header = bytearray()
    new_header += prefix
    new_header += file_version
    new_header += compare_hash_code
    new_header += n_procs
    new_header += struct.pack("@N", num_data)
    for count in new_n_data:
        new_header += struct.pack("@N", count)

    new_data = bytearray()
    for _, name, value_data, type_hash_code, type_name, has_context in entries:
        new_header += write_string(name)
        new_header += struct.pack("@N", len(value_data))
        new_header += type_hash_code
        new_header += write_string(type_name)
        new_header += has_context
        new_data += value_data

    header_path.write_bytes(new_header)
    data_path.write_bytes(new_data)


def remove_metadata(split_dir):
    shutil.rmtree(metadata_dir(split_dir))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("operation", choices=("drop-fingerprint", "remove-metadata"))
    parser.add_argument("split_dir")
    args = parser.parse_args()

    if args.operation == "drop-fingerprint":
        drop_fingerprint(args.split_dir)
    else:
        remove_metadata(args.split_dir)


if __name__ == "__main__":
    main()
