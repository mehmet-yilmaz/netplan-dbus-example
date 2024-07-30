#include "cli.hpp"
int main(int argc, char *argv[])
{

    const auto cli = std::make_unique<Cli>();
    if (cli)
        cli->run();
    return 0;
}
