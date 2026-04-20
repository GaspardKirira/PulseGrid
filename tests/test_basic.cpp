#include <vix/tests/tests.hpp>

int main()
{
  using namespace vix::tests;

  auto &registry = TestRegistry::instance();
  registry.clear();

  registry.add(TestCase("app basic test", [] {
    Assert::equal(2 + 2, 4);
  }));

  return TestRunner::run_all_and_exit();
}
