from sys import modules
from types import ModuleType
from .api import inject, attach, VitInjectError, LibraryNotFoundException, InjectorError


def legacy_vitinject_import():
    vitinject = ModuleType('vitinject.vitinject')
    vitinject.__package__ = 'vitinject'
    vitinject.InjectorError = InjectorError
    modules[vitinject.__name__] = vitinject


legacy_vitinject_import()

__all__ = ['inject', 'attach', 'VitInjectError', 'LibraryNotFoundException', 'InjectorError']
