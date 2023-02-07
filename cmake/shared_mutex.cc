#include <shared_mutex>

int main() {
    std::shared_mutex mut;
    std::shared_lock<std::shared_mutex> lock{mut};
}
