#include "graphics/application.hpp"

#include <cstdlib>
#include <iostream>
#include <string_view>

int main(int argc, char* argv[]) {
    triangles::TriangleSource source = triangles::TriangleSource::kInput;
    if (argc == 2 && std::string_view{argv[1]} == "--generate") source = triangles::TriangleSource::kGenerator;
    else if (argc > 2 || (argc == 2 && std::string_view{argv[1]} != "--input")) {
        std::cerr << "Usage: " << argv[0] << " [--input | --generate]\n";
        return EXIT_FAILURE;
    }

    triangles::Application app{source};
    return app.Run();
}
