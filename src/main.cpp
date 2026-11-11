#include "graphics/application.hpp"

#include <cstdlib>
#include <iostream>
#include <string_view>

int main(int argc, char* argv[]) {
    triangles::TriangleSource source = triangles::TriangleSource::kGenerator;
    if (argc == 2 && std::string_view{argv[1]} == "--input") source = triangles::TriangleSource::kInput;
    else if (argc > 2 || (argc == 2 && std::string_view{argv[1]} != "--generate")) {
        std::cerr << "Usage: " << argv[0] << " [--input | --generate]\n";
        return EXIT_FAILURE;
    }

    triangles::Application app{source};
    return app.Run();
}
