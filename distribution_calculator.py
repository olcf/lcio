import sys
import argparse
import math
import itertools


def convert_val(val):
    num_digits = len(str(val))
    divisor = 1
    if num_digits > 12:
        divisor = math.pow(1024, 4)
        suffix = 'TiB'
        return str(val/divisor) + suffix
    elif num_digits > 9:
        divisor = math.pow(1024, 3)
        suffix = 'GiB'
        return str(val/divisor) + suffix
    elif num_digits > 6:
        divisor = math.pow(1024, 2)
        suffix = 'MiB'
        return str(val/divisor) + suffix
    elif num_digits > 3:
        divisor = 1024
        suffix = 'KiB'
        return str(val/divisor) + suffix


def convert_suffix(val_bin):
    base = int(val_bin[:-1])
    suffix = val_bin[-1]
    mul = 1
    if suffix == 'k':
        mul = 1024
    elif suffix == 'm':
        mul = math.pow(1024, 2)
    elif suffix == 'g':
        mul = math.pow(1024, 3)
    elif suffix == 't':
        mul = math.pow(1024, 4)

    return base * int(mul)


def calc_params(dist_file):
    try:
        import configparser as ConfigParser
    except ImportError:
        import ConfigParser

    config = ConfigParser.ConfigParser()
    config.read(dist_file)
    bin_names = []
    counts = []

    for i, item in enumerate(config.items('dist')):
        bin_names.append(item[0])
        counts.append(int(item[1]))

    bins = list(map(convert_suffix, bin_names))
    sum_counts = sum(counts)

    raw_dist = list(map(lambda x: x/sum_counts, counts))
    bins_sqd = list(map(lambda x: x * x, bins))

    zipped_dist = zip(bins, raw_dist)
    zipped_dist_sqd = zip(bins_sqd, raw_dist)

    dist = list(itertools.starmap(lambda x, y: x*y, zipped_dist))
    dist_sqd = list(itertools.starmap(lambda x, y: x*y, zipped_dist_sqd))

    e_val = sum(dist)
    e_sqd = sum(dist_sqd)

    return round(e_val), round(e_sqd - (e_val * e_val))


def main(argv):
    parser = argparse.ArgumentParser(
        description="Helper tool to calculate correct parameters for a distribution")
    parser.add_argument('--file', metavar="DIST-FILE", nargs=1, type=str, help="distribution ini file from fprof")
    args = parser.parse_args(argv)

    e_val, var = calc_params(args.file[0])
    print("Expected size per file: ", convert_val(e_val))
    # print("Variance: ", convert_val(var))
    num_files = int(input("Num Files? "))
    epochs = int(input("Num Epochs? "))
    ops = int(input("Num Ops? "))

    print("Expected size needed: ", convert_val(num_files*e_val))
    print("Expected total traffic: ", convert_val(epochs*ops*e_val))

if __name__ == "__main__":
    main(sys.argv[1:])
