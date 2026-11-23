#include "parseJson.h"
#include <fstream>

using json = nlohmann::json;

json parseJsonFile()
{
    std::ifstream file(PHRASES_PATH);
    return json::parse(file);
}
