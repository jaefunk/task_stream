### Features

- task stream by lambda

# Example

``` cpp
int main(void)
{
    auto _A = [](int a, int b) { return a + b };
    auto _B = [](int ab) { return std::to_string(ab); };
    auto _C = [](std::string s) { return s.append("asdf"); };
    auto _R = task::create(_A) >> task::then(_B) >> task::then(_C);
	
    auto result1 = _R >> task::result(5, 10);
    // result1 is "15asdf"
	
    auto result2 = _R >> task::result(15, 25);
    // result2 is "40asdf"
}
```
