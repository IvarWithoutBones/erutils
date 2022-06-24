#include "arguments.h"
#include "savefile.h"
#include <fmt/format.h>

using namespace savepatcher;
using namespace savepatcher::arguments;

static void printActiveCharacters(std::vector<Character> &characters) {
    for (auto character : characters)
        if (character.active)
            fmt::print("  slot {}: {}, level {}, played for {}\n", character.getSlotIndex(), character.name, character.level, character.timePlayed);
    fmt::print("\n");
}

static void printUsage(const ArgumentParser &parser) {
    const auto [programName, briefUsage, fullUsage] = parser.getUsage();
    fmt::print("Usage: {} {}\n{}\n", programName, briefUsage, fullUsage);
}

int main(int argc, char **argv) {
    ArgumentParser arguments(argc, argv);
    arguments.add<std::string_view>({
        {"--from", "<save file>", "The path to the save file to copy from"},
        {"--to", "<save file>", "The path to the save file to copy to"},
        {"--read", "<save file>", "Print the slots of a save file"},
        {"--output", "The path to the output file to write to"},
    });

    arguments.add<int>({
        {"--append", "<slot number>", "Append character from the given slot to a save file. Both '--from' and '--to' need to be set"},
    });

    arguments.add<bool>({{"--help", "Show this help message"}});
    if (arguments.find<bool>("--help").set) {
        printUsage(arguments);
        exit(0);
    }

    const auto read{arguments.find<std::string_view>("--read")};
    if (read.set) {
        const auto path{std::filesystem::path{read.value}};
        auto saveFile{SaveFile{path}};
        fmt::print("Savefile '{}':\n", path.generic_string());
        printActiveCharacters(saveFile.slots);
        exit(0);
    }

    const auto sourcePath{arguments.find<std::string_view>("--from")};
    const auto targetPath{arguments.find<std::string_view>("--to")};
    if (!sourcePath.set || !targetPath.set) { // TODO: add support for required arguments
        printUsage(arguments);
        exit(1);
    }

    auto sourceSave{SaveFile(std::filesystem::path{sourcePath.value})};
    auto targetSave{SaveFile(std::filesystem::path{targetPath.value})};

    fmt::print("Savefile to copy from:\n");
    printActiveCharacters(sourceSave.slots);

    fmt::print("Savefile to copy to:\n");
    printActiveCharacters(targetSave.slots);

    auto appendArgument{arguments.find<int>("--append")};
    if (appendArgument.set) {
        if (sourceSave.slots[appendArgument.value].active)
            targetSave.appendSlot(sourceSave, appendArgument.value);
        else
            throw std::runtime_error(fmt::format("Attempting to append slot character {}", appendArgument.value));
    }

    fmt::print("Generated file:\n");
    printActiveCharacters(targetSave.slots);

    const auto outputArgument{arguments.find<std::string_view>("--output")};
    if (outputArgument.set) {
        const auto path{std::filesystem::path{outputArgument.value}};
        targetSave.write(path.generic_string());
        fmt::print("Succesfully wrote output to file '{}'\n", util::toAbsolutePath(path));
    }
}
