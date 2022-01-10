#include "state_machine.hpp"

int main(int argc, char const *argv[]) {
    auto d = coro_sm::get_door();
    d.on_event(coro_sm::open{}); // closed to open
    d.on_event(coro_sm::close{}); // open to close
    d.on_event(coro_sm::knock{});
    d.on_event(coro_sm::open{}); // closed to closed
    return 0;
}