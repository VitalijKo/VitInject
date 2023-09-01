import os
from contextlib import contextmanager
from .injector import Injector, InjectorException


class VitInjectError(Exception):
    pass


class LibraryNotFoundException(VitInjectError):
    def __init__(self, path):
        self.path = path

    def __str__(self):
        return f'Could not find library: {self.path}'


class InjectorError(VitInjectError):
    def __init__(self, func_name, ret_val, error_str):
        self.func_name = func_name
        self.ret_val = ret_val
        self.error_str = error_str

    def __str__(self):
        explanation = self.error_str or 'see error code definition in injector/include/injector.h'

        return f'Injector failed with {self.ret_val} calling {self.func_name}: {explanation}'


@contextmanager
def attach(pid):
    injector = Injector()
    injector.attach(pid)

    try:
        yield injector
    finally:
        injector.detach()


def inject(pid, library_path, uninject=False):
    if isinstance(library_path, str):
        encoded_library_path = library_path.encode()

    else:
        encoded_library_path = library_path

    assert isinstance(encoded_library_path, bytes)

    if not os.path.isfile(encoded_library_path):
        raise LibraryNotFoundException(encoded_library_path)

    with attach(pid) as injector:
        handle = injector.inject(encoded_library_path)

        if uninject:
            injector.uninject(handle)

        return handle
