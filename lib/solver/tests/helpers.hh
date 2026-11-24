#pragma once

//! @file
//! Helpers shared by the solver tests.

#include <clingo-lpx/parsing.hh>
#include <clingo-lpx/problem.hh>
#include <clingo-lpx/solving.hh>

#include <clingo/control.hh>
#include <clingo/core.hh>

#include <cstdint>
#include <memory>
#include <vector>

namespace ClingoLPX::Test {

Options const options{
    SelectionHeuristic::Conflict, StoreSATAssignments::Partial, std::nullopt, PropagateMode::Changed, true,
};

template <typename V = Rational> auto run(char const *s) -> bool {
    auto lib = Clingo::Library{};
    auto ctl = Clingo::Control{lib};
    register_theory(ctl);
    ctl.register_propagator(std::make_unique<Propagator<V>>(lib, options));
    ctl.parse_string(s);
    ctl.ground();
    return ctl.solve().satisfiable();
}

//! Portable deterministic random number generator (splitmix64).
class Random {
  public:
    explicit Random(uint64_t seed) : state_{seed} {}

    //! Return a number in the closed interval [lo, hi].
    auto operator()(fixed_int lo, fixed_int hi) -> fixed_int {
        state_ += 0x9e3779b97f4a7c15ULL;
        auto z = state_;
        z = (z ^ (z >> 30U)) * 0xbf58476d1ce4e5b9ULL;
        z = (z ^ (z >> 27U)) * 0x94d049bb133111ebULL;
        z ^= z >> 31U;
        return lo + static_cast<fixed_int>(z % static_cast<uint64_t>(hi - lo + 1));
    }

  private:
    uint64_t state_;
};

//! A linear constraint optionally guarded by an atom.
struct LPConstraint {
    std::vector<fixed_int> co;
    Relation rel;
    fixed_int rhs;
    fixed_int guard; //!< index of the guarding atom or -1
};

//! A small random linear program with a Boolean layer.
struct LP {
    std::vector<LPConstraint> cons;
    std::vector<fixed_int> obj;
    fixed_int atoms;
};

} // namespace ClingoLPX::Test
