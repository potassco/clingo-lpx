'''
A simplistic solver for checking the satisfiability of linear programs.
'''

from typing import Iterator, List, Mapping, Optional, Tuple, Union
from dataclasses import dataclass, field
from enum import Enum
from fractions import Fraction
from itertools import chain

Number = Fraction
Variable = str


class Operator(Enum):
    '''
    Enumeration of comparison operators.
    '''
    LessEqual = 0
    GreaterEqual = 1
    Equal = 2

    def invert(self) -> 'Operator':
        '''
        Flips the comparison operator.
        '''
        if self == Operator.LessEqual:
            return Operator.GreaterEqual
        if self == Operator.GreaterEqual:
            return Operator.LessEqual
        assert self == Operator.Equal
        return Operator.Equal

    def __str__(self) -> str:
        '''
        Return a string representation of the operator.
        '''
        if self.value is Operator.LessEqual.value:
            return '<='
        if self.value is Operator.GreaterEqual.value:
            return '>='
        assert self.value is Operator.Equal.value
        return '='

@dataclass
class Term:
    '''
    A term in form of a variable with a coefficient.
    '''
    co: Number
    var: Variable

    def __mul__(self, x: Number) -> 'Term':
        '''
        Multiply the coeffcient of the term with the given number.
        '''
        return Term(x * self.co, self.var)

    def __imul__(self, x: Number) -> 'Term':
        '''
        See __mul__.
        '''
        self.co *= x
        return self

    def __str__(self) -> str:
        '''
        Return a string representation of the term.
        '''
        if self.co == 1:
            return self.var
        if self.co == -1:
            return f'-{self.var}'
        if self.co < 0:
            return f'({self.co})*{self.var}'
        return f'{self.co}*{self.var}'

@dataclass
class Equation:
    '''
    An equation representing a sum of terms together with a comparison
    operator and a guard.
    '''
    lhs: List[Term]
    op: Operator
    rhs: Number

    def __mul__(self, x: Number) -> 'Equation':
        '''
        Multiply the terms and guard with the given number adjusting the
        comparison operator according to the sign of the number.
        '''
        return Equation(
            [y * x for y in self.lhs],
            self.op.invert() if x < 0 else self.op,
            x * self.rhs)

    def __imul__(self, x: Number) -> 'Equation':
        '''
        See __mul__.
        '''
        self.rhs = x * self.rhs
        for y in self.lhs:
            y *= x
        if x < 0:
            self.op = self.op.invert()
        return self

    def vars(self) -> Iterator[str]:
        '''
        An iterator for the variables occuring in the equation.
        '''
        for x in self.lhs:
            yield x.var

    def __str__(self) -> str:
        '''
        Return a string representation of the equation.
        '''
        lhs = ' + '.join(str(x) for x in self.lhs)
        return f'{lhs} {self.op} {self.rhs}'


@dataclass
class Solver:
    '''
    A problem in form of a set of equations that can be checked for
    satisfiability.
    '''
    equations: List[Equation]
    tableau: List[List[Number]] = field(default_factory=list)
    lower: Mapping[int, Number] = field(default_factory=dict)
    upper: Mapping[int, Number] = field(default_factory=dict)
    assignment: List[Number] = field(default_factory=list)
    variables: List[int] = field(default_factory=list)
    n_basic: int = 0
    n_pivots: int = 0

    def vars(self) -> List[str]:
        '''
        The set of all variables (in form of a list) appearing in the
        equations.
        '''
        return sorted(set(chain(
            chain.from_iterable(x.vars() for x in self.equations))))

    def check(self) -> bool:
        '''
        Check if the state invariants hold.
        '''
        for i, row_i in enumerate(self.tableau):
            v_i = 0
            for j, a_ij in enumerate(row_i):
                v_i += self.assignment[j] * a_ij

            if v_i != self.assignment[self.n_basic + i]:
                return False

        return True

    def pivot(self, i: int, j: int, v: Number) -> None:
        '''
        Pivots basic variable x_i and non-basic variable x_j.
        '''
        row_i = self.tableau[i]
        a_ij = row_i[j]
        assert a_ij != 0

        # adjust assignment
        ii = i + self.n_basic
        dj = (v - self.assignment[ii]) / a_ij
        self.assignment[ii] = self.assignment[j] + dj
        self.assignment[j] = v

        # invert row i
        for k, a_ik in enumerate(row_i):
            if k == j:
                row_i[j] = 1 / a_ij
            else:
                row_i[k] = a_ik / -a_ij

        # eliminate x_j from rows k != i
        for k, row_k in enumerate(self.tableau):
            a_kj = row_k[j]
            if k == i or a_kj == 0:
                continue
            v_k = Number(0)
            for l, a_kl in enumerate(row_k):
                if l == j:
                    row_k[j] = a_kj / a_ij
                else:
                    row_k[l] = a_kl + row_i[l] * a_kj
                v_k += row_k[l] * self.assignment[l]
            # at this point we can already check if the basic variable x_k is conflicting
            # and we can identify the best non-basic variable for pivoting
            # for not having to iterate in the select function
            # most notably we can already check if the problem is unsatisfiable here
            # if x_k is conflicting but cannot be adjusted, then the problem is unsatisfiable
            self.assignment[self.n_basic + k] = v_k

        # swap variables x_i and x_j
        self.variables[ii], self.variables[j] = self.variables[j], self.variables[ii]

        self.n_pivots += 1
        assert self.check()

    def select(self) -> Union[bool, Tuple[int, int, Number]]:
        '''
        Select pivot point using Bland's rule.
        '''
        basic = sorted(enumerate(self.variables[self.n_basic:]), key=lambda x: x[1])
        nonbasic = sorted(enumerate(self.variables[:self.n_basic]), key=lambda x: x[1])
        for i, xi in basic:
            axi = self.assignment[xi]

            li = self.lower.get(xi)
            if li is not None and axi < li:
                for j, xj in nonbasic:
                    aij = self.tableau[i][j]
                    axj = self.assignment[xj]
                    if aij > 0 and (xj not in self.upper or axj < self.upper[xj]):
                        return i, j, li
                    if aij < 0 and (xj not in self.upper or axj > self.lower[xj]):
                        return i, j, li

                return False

            ui = self.upper.get(xi)
            if ui is not None and axi > ui:
                for j, xj in nonbasic:
                    aij = self.tableau[i][j]
                    axj = self.assignment[xj]
                    if aij < 0 and (xj not in self.upper or axj < self.upper[xj]):
                        return i, j, ui
                    if aij > 0 and (xj not in self.upper or axj > self.lower[xj]):
                        return i, j, ui

                return False

        return True

    def prepare(self) -> None:
        '''
        Prepare equations for solving.
        '''
        variables = self.vars()
        indices = {x: i for i, x in enumerate(variables)}
        n = len(variables)
        m = len(self.equations)

        self.n_basic = n
        self.assignment = (m + n) * [Number(0)]

        self.tableau = []
        lower = dict()
        upper = dict()
        for i, x in enumerate(self.equations):
            self.tableau.append(n * [Number(0)])
            for y in x.lhs:
                self.tableau[-1][indices[y.var]] += y.co

            if x.op == Operator.LessEqual:
                upper[n + i] = x.rhs
            elif x.op == Operator.GreaterEqual:
                lower[n + i] = x.rhs
            elif x.op == Operator.Equal:
                upper[n + i] = x.rhs
                lower[n + i] = x.rhs
        self.lower = lower
        self.upper = upper

        self.variables = list(range(0, m+n))

        self.n_pivots = 0

    def solve(self) -> Optional[List[Tuple[str, Number]]]:
        '''
        Solve the (previously prepared) problem.
        '''
        while True:
            p = self.select()
            if p is True:
                return [(var, self.assignment[self.variables[i]])
                        for i, var in enumerate(self.vars())]
            if p is False:
                return None

            assert isinstance(p, tuple)
            self.pivot(*p)

    def debug(self) -> str:
        '''
        Return a debug representation of the internal state.
        '''
        ret = ' '.join(f'{str(x)}={self.assignment[x]}' for x in self.variables)
        for row in self.tableau:
            ret += '\n' + ' '.join(f'{str(x):2}' for x in row)
        ret += f'\nsat: {self.select() is True}'
        return ret

    def __str__(self) -> str:
        '''
        Print the equations in the problem.
        '''
        return '\n'.join(f'{x}' for x in self.equations)


def run(e: List[Equation]) -> None:
    '''
    Solve the given problem and print result.
    '''
    p = Solver(e)
    print('Problem :')
    print(p)
    p.prepare()
    ret = p.solve()
    if ret is not None:
        print('Solution:', ' '.join(f'{var}={val}' for var, val in ret))
        print('Result  : SAT')
    else:
        print('Result  : UNSAT')
    print('Pivots  :', p.n_pivots)


if __name__ == '__main__':
    run([Equation([Term(Number(1), 'x1'), Term(Number(1), 'x2')], Operator.LessEqual, Number(20)),
         Equation([Term(Number(1), 'x1'), Term(Number(1), 'x3')], Operator.Equal, Number(5)),
         Equation([Term(Number(-1), 'x2'), Term(Number(-1), 'x3')], Operator.LessEqual, Number(-10))])
    print()

    run([Equation([Term(Number(1), 'x')], Operator.GreaterEqual, Number(2)),
         Equation([Term(Number(2), 'x')], Operator.LessEqual, Number(0))])
    print()

    run([Equation([Term(Number(1), 'x'), Term(Number(1), 'y')], Operator.GreaterEqual, Number(2)),
         Equation([Term(Number(2), 'x'), Term(Number(-1), 'y')], Operator.GreaterEqual, Number(0)),
         Equation([Term(Number(-1), 'x'), Term(Number(2), 'y')], Operator.GreaterEqual, Number(1))])
