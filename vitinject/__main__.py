import sys
from argparse import ArgumentParser
from vitinject import inject
from vitinject.api import VitInjectError


def parse_args(args):
    parser = ArgumentParser(description='Inject a dynamic library to a running process.')
    parser.add_argument('pid', type=int, help='PID of the process to inject the library into')
    parser.add_argument('library_path', type=str.encode, help='Path of the library to inject')
    parser.add_argument('-u', '--uninject', action='store_true', help='Whether to uninject the library after injection')

    return parser.parse_args(args)


def main(args=None):
    args = parse_args(args)

    try:
        handle = inject(args.pid, args.library_path, args.uninject)
    except VitInjectError as e:
        print(f'VitInject failed to inject: {e}', file=sys.stderr)
    else:
        print(f'Handle: {handle}')
