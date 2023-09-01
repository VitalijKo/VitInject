from setuptools import setup, Extension
from pathlib import Path

PROJECT_ROOT = Path(__file__).parent.resolve()
INJECTOR_DIR = PROJECT_ROOT / 'injector'
INJECTOR_SRC = INJECTOR_DIR / 'src'
INJECTOR_WRAPPER = PROJECT_ROOT / 'vitinject' / 'injector.c'
SOURCES = [str(c.relative_to(PROJECT_ROOT))
           for c in [INJECTOR_WRAPPER, *INJECTOR_SRC.iterdir()]
           if c.suffix == '.c']

injector_extension = Extension(
    'vitinject.injector',
    sources=SOURCES,
    include_dirs=[str(INJECTOR_DIR.relative_to(PROJECT_ROOT) / 'include')],
    export_symbols=['injector_attach', 'injector_inject', 'injector_detach', 'injector_error'],
    define_macros=[('EM_AARCH64', '183')]
)

setup(
    ext_modules=[injector_extension],
    entry_points={'console_scripts': ['inject=vitinject.__main__:main']}
)
