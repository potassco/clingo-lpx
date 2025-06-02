'''
This module provides a clingo.Theory class for a LPX theory.
'''

from clingo.theory import Theory
from ._clingolpx import lib as _lib, ffi as _ffi

__all__ = ['ClingoLPXTheory']

class ClingoLPXTheory(Theory):
    '''
    The DL theory.
    '''
    def __init__(self):
        super().__init__("clingolpx", _lib, _ffi)
