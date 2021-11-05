#!/usr/bin/python
'''
This module provides an application class similar to clingo-lpx plus a main
function to execute it.
'''

import sys

import clingo
from clingo import ast
from . import ClingoLPXTheory

class Application(clingo.Application):
    '''
    Application class similar to clingo-lpx (excluding optimization).
    '''
    def __init__(self, name):
        self.__theory = ClingoLPXTheory()
        self.program_name = name
        self.version = ".".join(str(x) for x in self.__theory.version())

    def register_options(self, options):
        self.__theory.register_options(options)

    def validate_options(self):
        self.__theory.validate_options()
        return True

    def print_model(self, model, printer):
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

    def main(self, control, files):
        self.__theory.register(control)

        with ast.ProgramBuilder(control) as bld:
            ast.parse_files(files, lambda stm: self.__theory.rewrite_ast(stm, bld.add))

        control.ground([("base", [])])
        self.__theory.prepare(control)

        control.solve(on_model=self.__on_model, on_statistics=self.__on_statistics)

    def __on_model(self, model):
        self.__theory.on_model(model)

    def __on_statistics(self, step, accu):
        self.__theory.on_statistics(step, accu)

if __name__ == "__main__":
    sys.exit(int(clingo.clingo_main(Application("clingo-lpx"), sys.argv[1:])))
