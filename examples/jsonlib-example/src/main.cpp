#include <adhan/MathUtils.hpp>
#include <fstream>
#include <iostream>
#include <json.hpp>

using json = nlohmann::json;
int main() {
    std::ifstream f("examples/jsonlib-example/dummy.json");
    json data = json::parse(f);
    std::cout << "::::::::::: PLAIN OBJECT ::::::::::::" << std::endl;
    std::cout << data["params"].dump(2) << std::endl;
    std::cout << ":::::::::: LIST OF OBJECTS ::::::::::" << std::endl;
    std::cout << "[" << std::endl;
    for (int i = 0; i < 3; i++) {
        std::cout << data["times"][i].dump(2);
        if (i < 2) std::cout << ",";
        std::cout << std::endl;
    }
    std::cout << "]" << std::endl;
    std::cout << ":::::::::::::::: END ::::::::::::::::" << std::endl;
    return 0;
}