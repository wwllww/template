// AutoTuiDao.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <tuple>
#include <iostream>
#include <string>
#include <utility>
#include <functional>
#include <unordered_map>
#include <memory>
#include <map>
#include <type_traits>

int func1(int arg1, char arg2, double arg3, const std::string& arg4)
{
	std::cout << "call func1(" << arg1 << ", " << arg2 << ", " << arg3 << ", " << arg4 << ")" << std::endl;
	return 2;
}

int func2(int arg1, int arg2)
{
	std::cout << "call func2(" << arg1 << ", " << arg2 << ")" << std::endl;
	return arg1 + arg2;
}

// template<typename F, typename T, std::size_t... I>
// auto apply_impl(F f, const T& t, std::index_sequence<I...>) -> decltype(f(std::get<I>(t)...))
// {
// 	return f(std::get<I>(t)...);
// }
// 
// template<typename F, typename T>
// auto apply(F f, const T& t) -> decltype(apply_impl(f, t, std::make_index_sequence<std::tuple_size<T>::value>()))
// {
// 	return apply_impl(f, t, std::make_index_sequence<std::tuple_size<T>::value>());
// }

template <typename R, typename... Args>
std::function<R(Args...)> cache(R(*func) (Args...))
{
	auto result_map = std::make_shared<std::map<std::tuple<Args...>, R>>();

	return ([=](Args... args){
		std::tuple<Args...> t(args...);
		if (result_map->find(t) == result_map->end())
			(*result_map)[t] = func(args...);

		return (*result_map)[t];
	});
}

template <typename R, typename...  Args>
std::function<R(Args...)> sugar(R(*func)(Args...), bool needClear = false)
{
	using function_type = std::function<R(Args...)>;
	static std::unordered_map<decltype(func), function_type> functor_map;

	if (needClear)
		return functor_map[func] = cache(func);

	if (functor_map.find(func) == functor_map.end())
		functor_map[func] = cache(func);

	return functor_map[func];
}

template <class... T>
void f(T... args)
{
	std::cout << sizeof...(args) << std::endl; //打印变参的个数
}


//整型序列的定义
template<int...>
struct IndexSeq{};

//继承方式，开始展开参数包
template<int N, int... Indexes>
struct MakeIndexes : MakeIndexes<N - 1, N - 1, Indexes...> {};

// 模板特化，终止展开参数包的条件
template<int... Indexes>
struct MakeIndexes<0,Indexes...>
{
	//typedef IndexSeq<Indexes...> type;
	using type = IndexSeq<Indexes...>;
};

template<typename F, int ... Indexes, typename ... Args>
static auto call_helper(F f, IndexSeq<Indexes...>, const std::tuple<Args...>& tup) -> decltype(f(std::get<Indexes>(tup)...))
{
	return f(std::get<Indexes>(tup)...);
}

template<typename F, typename ... Args>
static auto call(F f, const std::tuple<Args...>& tp) ->decltype(call_helper(f, MakeIndexes<sizeof... (Args)>::type(), tp))
{
	return call_helper(f, MakeIndexes<sizeof... (Args)>::type(), tp);
}


template<int ... Indexes, typename ... Args>
std::vector<int> call_helper2(IndexSeq<Indexes...>, const std::tuple<Args...>& tup)
{
	return{ std::get<Indexes>(tup)... };
}

template <typename ...Args, typename T>
auto GetList(const std::tuple<Args...>& tp, T t) ->decltype(call_helper2(MakeIndexes<sizeof... (Args)>::type(), tp))
{
	return call_helper2(MakeIndexes<sizeof... (Args)>::type(), tp);
}

///////////////////////////////////////////
struct Hello
{
	typedef int INT;
	void operator()() { std::cout << "Hello World" << std::endl; }
};
struct Generic {
};

#define HAS_MEM_FUNC(FunctionName, HelperClassName) \
template<typename T> \
struct HelperClassName {\
typedef char one; \
typedef long two; \
template <typename C> static one test(decltype(C::FunctionName)); \
template <typename C> static two test(...); \
enum { value = sizeof(test<T>(0)) == sizeof(char) };\
}


HAS_MEM_FUNC(operator(), hasOperatorParentheses);

template<class T, typename = void>
class A
{
public:
	static const bool value = false;
	static void execute(const typename std::decay<T>::type & t){ std::cout << "no function to call" << std::endl; }
};

template<class T>
class A<T, typename std::enable_if<hasOperatorParentheses<T>::value>::type>
{
public:
	static const bool value = true;
// 	static void execute(const typename std::decay<T>:t:type & t){ 
// 		const_cast<typename std::decay<T>::type&>(t)();

	static void execute(T & t){
		t();
	}
};
//////////////////////////////////////////////////////////////////

int Functin(int a, int b)
{
	return a + b;
}

std::string Funcc(int a, int b)
{
	return std::string("12121211");
}

int _tmain(int argc, _TCHAR* argv[])
{
	//using namespace std::literals::string_literals;
	std::tuple<int, char, double, const char*> tuple1 = std::make_tuple(1, 'A', 1.2, "破晓的博客");
	//auto result1 = apply(func1, tuple1);
// 	auto result1 = sugar(func1)(std::get<0>(tuple1), std::get<1>(tuple1), std::get<2>(tuple1), std::get<3>(tuple1));
// 	std::cout << "result1 = " << result1 << std::endl;

	call(func1, tuple1);

	auto tuple11 = std::make_tuple(1, 3);

	std::cout << call(Funcc, tuple11).c_str() << std::endl;

 	auto tuple2 = std::make_tuple(1, 2);
	int tuple3 = std::get<0>(tuple2);
	int sum = call(func2, tuple2);

// 	auto result2 = apply(func2, tuple2);
// 	std::cout << "result2 = " << result2 << std::endl;

	f();        //0
	f(1, 2);    //2
	f(1, 2.5, "");    //3

	auto tuple4 = std::make_tuple(1, 2,4,6,7);
	std::vector<int> &Listint = GetList(tuple4,5);
	//std::vector<int> &Listsize = GetList(tuple1);

	 A<Hello>::execute(Hello());
	 A<Generic>::execute(Generic());
	
	 std::cout << typeid(decltype(Hello::operator())).name() << std::endl;
	 std::cout << typeid(int).name() << std::endl;
	 bool IsSame = std::is_same<typename decltype(Hello::operator()), uint64_t>::value;

	getchar();
	return 0;
}


