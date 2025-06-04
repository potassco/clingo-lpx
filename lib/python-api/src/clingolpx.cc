#include <clingo-lpx.h>

#include <pybind11/pybind11.h>

namespace PyClingoLPX {

namespace {

auto create_theory() -> pybind11::object {
    return pybind11::capsule{reinterpret_cast<void *>(&clingolpx_create), "clingo_theory_create"};
}

} // namespace

void register_clingolpx(pybind11::module &m) {
    m.doc() = R"doc(TODO)doc";
    m.def("create_theory", create_theory, R"(TODO)");
}

} // namespace PyClingoLPX

PYBIND11_MODULE(clingolpx, m) { PyClingoLPX::register_clingolpx(m); }
