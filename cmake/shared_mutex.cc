#include <shared_mutex>

auto main() -> int {
    std::shared_mutex mut;
    std::shared_lock<std::shared_mutex> lock{mut};
}
