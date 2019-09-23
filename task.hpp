#pragma once

#include <functional>
#include <tuple>

namespace function_detail {
    template<class ret, class cls, class _is_mutable, class ...args> struct types {
	typedef _is_mutable is_mutable;
	typedef ret return_type;
	typedef std::function<ret(args...)> function_type;
	enum { arity = sizeof...(args) };
	template<std::size_t index>
	struct arguments {
            static_assert(index < arity, "error: invalid parameter index.");
            typedef typename std::tuple_element<index, std::tuple<args...>>::type type;
	};
    };
}
template<class fty>
struct function_trait : function_trait<decltype(&fty::operator())> {};
template<class ret, class cls, class... args>
struct function_trait<ret(cls::*)(args...)> : function_detail::types<ret, cls, std::true_type, args...> {};
template<class ret, class cls, class... args>
struct function_trait<ret(cls::*)(args...) const> : function_detail::types<ret, cls, std::false_type, args...> {};

namespace task {
    namespace details {
        template<typename task_builder, typename task> struct builtup_type {
            static task_builder get_builder();
	    static task get_task();
	    typedef decltype (get_builder().build(get_task())) type;
        };

	template <typename arty, typename prty> struct process {
            template <typename ar, typename pr, typename ...args>
            static prty call(ar _ar, pr _pr, args... arg) { return _pr(_ar.value(arg...)); }
	};
	template <typename arty> struct process <arty, void> {
            template <typename ar, typename pr, typename ...args>
            static void call(ar _ar, pr _pr, args... arg) { _pr(_ar.value(arg...)); }
	};
	template <typename prty> struct process <void, prty> {
            template <typename ar, typename pr, typename ...args>
            static prty call(ar _ar, pr _pr, args... arg) { _ar.value(arg...); return _pr(); }
	};
	template <> struct process <void, void> {
            template <typename ar, typename pr, typename ...args>
            static void call(ar _ar, pr _pr, args... arg) { _ar.value(arg...); _pr(); }
	};

	struct base_task {};
	struct base_builder {};

	template <typename predicate_type>
	struct create_task : public base_task {
    	    typedef create_task<predicate_type> this_type;
    	    typedef typename function_trait<predicate_type>::return_type return_type;
            predicate_type predicate;
	    create_task(predicate_type predicate) : predicate(std::move(predicate)) {}
	    create_task(create_task const & v) : predicate(v.predicate) {}
	    create_task(create_task && v) : predicate(std::move(v.predicate)) {}
	    template<typename task_builder>
	    typename builtup_type<task_builder, this_type>::type operator >> (task_builder _task_builder) const {
		return _task_builder.build(*this);
	    }
	    template <typename ty = return_type, typename ...args> return_type value(args... arg) {
		return predicate(arg...);
	    }
	};

	template <typename predicate_type, typename argument_type>
	struct then_task : public base_task {
	    typedef then_task<predicate_type, argument_type> this_type;
            typedef typename function_trait<predicate_type>::return_type return_type;
	    predicate_type predicate;
	    argument_type param;
	    explicit then_task(predicate_type predicate, argument_type param) : predicate(std::move(predicate)), param(std::move(param)) {}
	    then_task(then_task const & v) : predicate(v.predicate), param(v.param) {}
	    then_task(then_task && v) : predicate(std::move(v.predicate)), param(std::move(v.param)) {}
	    template<typename task_builder>
	    typename builtup_type<task_builder, this_type>::type operator >> (task_builder _task_builder) const {
		return _task_builder.build(*this);
            }
            template <typename rty = typename argument_type::return_type, typename ...args>
            const return_type value(args... arg) {
	        return process<rty, return_type>::call(param, predicate, arg...);
            }
            template <>
            const return_type value<void>(void) { 
	        param.value();
                return predicate();
            }
        };

	template <typename predicate_type>
	struct then_builder : public base_builder {
            typedef then_builder<predicate_type> this_type;
            typedef predicate_type function_type;
            typedef typename function_trait<predicate_type>::return_type predicate_return_type;
            function_type predicate;
            explicit then_builder(function_type predicate) : predicate(std::move(predicate)) {}
            then_builder(then_builder const & v) : predicate(v.predicate) {}
            then_builder(then_builder && v) : predicate(std::move(v.predicate)) {}
            template<typename argument_type>
            then_task<predicate_type, argument_type> build(argument_type param) const {
                return then_task<predicate_type, argument_type>(std::move(predicate), param);
            }
	};
        
	template <std::size_t arguments_cnt, typename ...args>
	struct result_builder : public base_builder {
            typedef std::tuple<args...> arguments_type;
            arguments_type arguments;
            explicit result_builder(args... arg) : arguments(std::forward<args>(arg)...) {}
            template <typename task_type, std::size_t... is>
            const typename task_type::return_type task_call(task_type task, const arguments_type _arguments, std::index_sequence<is...>) const {
                return task.value(std::get<is>(_arguments)...);
            }
	    template<typename task_type>
	    const typename task_type::return_type build(task_type task) const {
                return task_call(task, arguments, std::index_sequence_for<args...>());
            }
	};
        
        template <typename ...args>
        struct result_builder<0, args...> : public base_builder {
            typedef result_builder<0, args...> this_type;
            explicit result_builder(args... arg) {}
            template<typename task_type>
            const typename task_type::return_type build(task_type task) const {
                return task.value();
            }
        };
    }
	
    template <typename predicate_type>
    details::create_task<predicate_type> create(predicate_type predicate) {
	return details::create_task<predicate_type>(std::move(predicate));
    }

    template <typename predicate_type>
    details::then_builder<predicate_type> then(predicate_type predicate) {
	return details::then_builder<predicate_type>(std::move(predicate));
    }

    template <typename ...args>
    details::result_builder<sizeof...(args), args...> result(args... arg) {
	return details::result_builder<sizeof...(arg), args...>(arg...);
    }
};
