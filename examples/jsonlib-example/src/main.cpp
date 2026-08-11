#include <iostream>
#include <json.hpp>

using json = nlohmann::json;

// CONTAINS TRUNCATED DATA, AND IS ONLY TO BE USED AS A PLACEHOLDER
namespace {

constexpr const char *kDummyData = R"json(
{
  "params": {
    "latitude": 39.939382,
    "longitude": 32.819713,
    "timezone": "Europe/Istanbul",
    "method": "Turkey",
    "madhab": "Shafi",
    "highLatitudeRule": "MiddleOfTheNight"
  },
  "variance": 2,
  "times": [
    {
      "date": "2019-01-01",
      "fajr": "6:33 AM",
      "sunrise": "8:03 AM",
      "dhuhr": "12:57 PM",
      "asr": "3:20 PM",
      "maghrib": "5:40 PM",
      "isha": "7:05 PM"
    },
    {
      "date": "2019-01-15",
      "fajr": "6:33 AM",
      "sunrise": "8:02 AM",
      "dhuhr": "1:03 PM",
      "asr": "3:32 PM",
      "maghrib": "5:54 PM",
      "isha": "7:17 PM"
    },
    {
      "date": "2019-02-01",
      "fajr": "6:25 AM",
      "sunrise": "7:51 AM",
      "dhuhr": "1:07 PM",
      "asr": "3:49 PM",
      "maghrib": "6:14 PM",
      "isha": "7:34 PM"
    }
  ]
}
)json";

} // namespace

int main() {
  json data = json::parse(kDummyData);
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