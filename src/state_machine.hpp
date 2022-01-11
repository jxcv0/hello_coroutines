#ifndef STATE_MACHINE_HPP
#define STATE_MACHINE_HPP

#include <coroutine>
#include <iostream>
#include <variant>
#include <any>

namespace coro_sm {

    // events
    template<typename... events>
    struct event {};

    struct open {};
    struct close {};
    struct knock {};

    // promise type
    struct state_machine {
        struct promise_type {
            using handle = std::coroutine_handle<promise_type>;
            
            state_machine get_return_object() noexcept {
                return { handle::from_promise(*this) };
            }

            std::suspend_never initial_suspend() const noexcept { return {}; }
            std::suspend_always final_suspend() const noexcept { return {}; }

            template<typename... E>
            auto await_transform(event<E...>) noexcept {
                is_wanted_event = [](const std::type_info& type)->bool {
                    return ((type == typeid(E)) || ...);
                };

                // awaitable
                struct awaitable { // TODO - what does this do?

                    bool await_ready() const noexcept { return false; }
                    void await_suspend(handle) noexcept {}

                    std::variant<E...> await_resume() const {
                        std::variant<E...> event;
                        (void) ((
                            current_event->type() == typeid(E) ?
                            (event = std::move(*std::any_cast<E>(current_event))
                            , true) : false) || ...);
                        return event;
                    }
                    const std::any *current_event;
                };

                return awaitable { &current_event };
            }

            void return_void() noexcept {}
            void unhandled_exception() noexcept {}

            std::any current_event;
            bool (*is_wanted_event)(const std::type_info&) /* = nullptr */ ;
        };

        ~state_machine() { coro.destroy(); }
        state_machine(const state_machine &) = delete;
        state_machine &operator=(const state_machine &) = delete;

        // on event
        template<typename E>
        void on_event(E &&e) {
            auto& promise = coro.promise();
            if (promise.is_wanted_event(typeid(E))) {
                promise.current_event = std::forward<E>(e);
                coro();
            }
        }

    private:
        state_machine(std::coroutine_handle<promise_type> coro) : coro { coro } {}
        std::coroutine_handle<promise_type> coro;
    };

    state_machine get_door(std::string ans) {
        for(;;) {
            // closed
            auto e = co_await event<open, knock>{};
            if (std::holds_alternative<knock>(e)) {
                std::cout << ans << "\n";
            } else if (std::holds_alternative<open>(e)) {
                // open
                co_await event<close>{};
            }
        }
    };
}

#endif