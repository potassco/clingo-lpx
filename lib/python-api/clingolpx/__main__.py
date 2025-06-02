#!/usr/bin/python
'''
This module provides an application class similar to clingo-lpx plus a main
function to execute it.
'''

import sys
from typing import Callable, Sequence

from clingo import ast
from clingo.application import Application, ApplicationOptions, Flag, clingo_main
from clingo.control import Control
from clingo.script import enable_python
from clingo.solving import Model
from clingo.statistics import StatisticsMap

from . import ClingoLPXTheory


class ClingoLPXApp(Application):
    '''
    Application class similar to clingo-lpx (excluding optimization).
    '''
    def __init__(self, name: str):
        self._enable_python = Flag()
        self.__theory = ClingoLPXTheory()
        self.program_name = name
        self.version = ".".join(str(x) for x in self.__theory.version())

    def register_options(self, options: ApplicationOptions):
        """
        Register clingo-lpx related options.
        """
        options.add_flag("Basic Options", "enable-python", "Enable Python script tags", self._enable_python)
        self.__theory.register_options(options)

    def validate_options(self) -> bool:
        """
        Validate options.
        """
        self.__theory.validate_options()
        return True

    def print_model(self, model: Model, printer: Callable[[], None]):
        """
        Print assignment along with model.
        """
        # print model
        symbols = model.symbols(shown=True)
        sys.stdout.write(" ".join(str(symbol) for symbol in sorted(symbols)))
        sys.stdout.write('\n')

        # print assignment
        sys.stdout.write('Assignment:\n')
        symbols = model.symbols(theory=True)
        assignment = []
        for symbol in sorted(symbols):
            if symbol.match("__lpx", 2):
                args = symbol.arguments
                assignment.append(f"{args[0]}={args[1].string}")
        sys.stdout.write(" ".join(assignment))
        sys.stdout.write('\n')

        sys.stdout.flush()

    def main(self, control: Control, files: Sequence[str]):
        """
        Run clingcon application.
        """
        if self._enable_python:
            enable_python()

        self.__theory.register(control)

        with ast.ProgramBuilder(control) as bld:
            ast.parse_files(files, lambda stm: self.__theory.rewrite_ast(stm, bld.add))

        control.ground([("base", [])])
        self.__theory.prepare(control)

        control.solve(on_model=self.__on_model, on_statistics=self.__on_statistics)

    def __on_model(self, model: Model):
        """
        Pass model to theory.
        """
        self.__theory.on_model(model)

    def __on_statistics(self, step: StatisticsMap, accu: StatisticsMap):
        """
        Pass statistics to theory.
        """
        self.__theory.on_statistics(step, accu)


if __name__ == "__main__":
    sys.exit(int(clingo_main(ClingoLPXApp("clingo-lpx"), sys.argv[1:])))
