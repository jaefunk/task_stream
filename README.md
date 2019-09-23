### Features

- task stream by lambda

# Example
####c++
```
int main(void)
{
	auto _A = [](int a, int b) { return a + b };
	auto _B = [](int ab) { return std::to_string(ab); };
	auto _C = [](std::string s) { return s.append("asdf"); };

	auto _R = task::create(_A) >> task::then(_B) >> task::then(_C) >> task::result(5, 10);
	// _R is "15asdf"
}
```
