# thread_pool
C++11 thread pool for everyday use.

## Requirements
- [msvc](https://visualstudio.microsoft.com/) **>= 2022**

## Installation

- thread_pool is a header only library.
```cpp
#include "thread_pool.hpp"
```

## Basic Usage
- You can queue any function with any parameter with any return type. Enqueue and forget.

```cpp
#include "thread_pool.hpp"
#include <iostream>
#include <thread>
#include <format>
#include <functional>

int squared(int input) {
	return input * input;
}

std::string float_to_string(float input) {
	return std::format("{:02.4f}", input);
}

class square {
	int a_;
public:
	square(int a) : a_(a) {}
	int area() { return a_ * a_; }
};

int main()
{
	thread_pool tx;
	square abcd(5);

	auto r1 = tx.enqueue(squared, 2);
	auto r2 = tx.enqueue(float_to_string, 2.4f);
	auto r3 = tx.enqueue([&abcd] { return abcd.area(); });

	std::this_thread::sleep_for(std::chrono::seconds(1));
	
	if (r1.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
		std::cout << r1.get();
	}

	return 0;
}

```
