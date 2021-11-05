import sys
import site
from os.path import dirname, abspath
from textwrap import dedent
from skbuild import setup
import clingo


if not site.ENABLE_USER_SITE and "--user" in sys.argv[1:]:
    site.ENABLE_USER_SITE = True

clingopath = abspath(dirname(clingo.__file__))

setup(
    version = '1.0.0',
    name = 'clingo-lpx',
    description = 'CFFI-based bindings to the clingo-lpx solver.',
    long_description = dedent('''\
        This package allows for adding the clingo-lpx propagator as a
        theory to clingo.

        It can also be used as a clingo-lpx solver running:

            python -m clingolpx CLINGOLPX_ARGUMENTS
        '''),
    long_description_content_type='text/markdown',
    author = 'Roland Kaminski',
    author_email = 'kaminski@cs.uni-potsdam.de',
    license = 'MIT',
    url = 'https://github.com/potassco/clingo-lpx',
    install_requires=[ 'cffi', 'clingo' ],
    cmake_args=[ '-DCLINGOLPX_MANAGE_RPATH=OFF',
                 '-DPYCLINGOLPX_ENABLE=pip',
                 '-DPYCLINGOLPX_INSTALL_DIR=libpyclingo-lpx',
                 f'-DPYCLINGOLPX_PIP_PATH={clingopath}' ],
    packages=[ 'clingolpx' ],
    package_data={ 'clingolpx': [ 'py.typed', 'import__clingo-lpx.lib', 'clingo-lpx.h' ] },
    package_dir={ '': 'libpyclingo-lpx' },
    python_requires=">=3.6"
)
