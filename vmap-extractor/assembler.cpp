#include "TileAssembler.h"
#include <string>

bool AssembleVMAP(std::string src, std::string dest, const char* szMagic)
{
    bool success = true;
    VMAP::TileAssembler* ta = new VMAP::TileAssembler(src, dest);

    if (!ta->convertWorld2(szMagic))
        { success = false; }

    delete ta;
    return success;
}
