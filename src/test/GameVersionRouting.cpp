#include <cassert>

#include "GameVersionList.h"

int main() {
    assert(!game_versions::usesNeoForge("1.20.1"));
    assert(!game_versions::usesNeoForge("1.21.7"));
    assert(game_versions::usesNeoForge("1.21.8"));
    assert(game_versions::usesNeoForge("1.21.10"));
    assert(game_versions::usesNeoForge("1.22.0"));
    assert(!game_versions::usesNeoForge("invalid"));
    assert(!game_versions::usesNeoForge("1.21.8.1"));
    return 0;
}