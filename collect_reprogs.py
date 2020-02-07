#!/usr/bin/env python3

import argparse
import glob
import filecmp
import logging
import os
import shutil


def main():
    parser = argparse.ArgumentParser(description='Collect C reproducers programs from subdirs.')
    parser.add_argument('directory', type=str,
                        help='Folder to be searched for reprogs.')
    parser.add_argument('--output', '-o', required=False, default="./linux",
                        help='Output folder where to store. default ./linux')
    args = parser.parse_args()

    workdir = os.path.join(args.directory, "")
    logging.debug(workdir)

    for file in glob.iglob(workdir + '**/*.cprog', recursive=True):
        logging.debug(file)
        file_path = file.split('/')
        has_id_index = len(file_path) - 2

        candidate_name = file_path[has_id_index]
        candidate_file_name = os.path.join(args.output, candidate_name + ".c")

        if os.path.exists(candidate_file_name):
            logging.info("Reproducer: " + file + " already exists.")
            continue

        logging.info("Added new reprog: " + candidate_file_name)
        if not os.path.exists(args.output):
            os.mkdir(args.output)
        shutil.copy(file, candidate_file_name)


if __name__ == "__main__":
    logging.basicConfig(format='%(levelname)s: %(message)s', level=logging.INFO)
    main()
