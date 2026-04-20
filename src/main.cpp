#include <vix.hpp>
using namespace vix;

int main()
{
  App app;

  // GET /
  app.get("/", [](Request&, Response& res) {
    res.send("Hello world");
  });

  app.run(8080);
}
