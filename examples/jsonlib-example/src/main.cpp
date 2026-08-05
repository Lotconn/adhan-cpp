#include <adhan/MathUtils.hpp>
#include <fstream>
#include <iostream>
#include <json.hpp>

using json = nlohmann::json;
int main() {
  std::ifstream f("examples/jsonlib-example/dummy.json");
  json data = json::parse(f);
  std::cout << "::::::::::: PLAIN OBJECT ::::::::::::" << '\n';
  std::cout << data["params"].dump(2) << '\n';
  std::cout << ":::::::::: LIST OF OBJECTS ::::::::::" << '\n';
  std::cout << "[" << '\n';
  for (int i = 0; i < 3; i++) {
    std::cout << data["times"][i].dump(2);
    if (i < 2) {
      std::cout << ",";
    }
    std::cout << '\n';
  }
  std::cout << "]" << '\n';
  std::cout << ":::::::::::::::: END ::::::::::::::::" << '\n';
  return 0;
}