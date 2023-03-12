/**
 * MaNGOS is a full featured server for World of Warcraft, supporting
 * the following clients: 1.12.x, 2.4.3, 3.3.5a, 4.3.4a and 5.4.8
 *
 * Copyright (C) 2005-2023 MaNGOS <https://getmangos.eu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * World of Warcraft, and all World of Warcraft or Warcraft art, images,
 * and lore are copyrighted by Blizzard Entertainment, Inc.
 */

#include "vmapexport.h"
#include "wmo.h"
#include "vec3d.h"
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <map>
#include <fstream>
#include <ExtractorCommon.h>
#undef min
#undef max

extern uint16* LiqType;
extern bool preciseVectorData;
extern ArchiveSet gOpenArchives;

WMORoot::WMORoot(std::string& filename) : filename(filename)
{
}

bool WMORoot::open()
{
    HANDLE mpqFile;
    if (!OpenNewestFile(filename.c_str(), &mpqFile))
    {
        printf("Error opening WMO Root %s\n", filename.c_str());
    }
    MPQFile f(mpqFile, filename.c_str());
    if (f.isEof())
    {
        printf(" No such file %s.\n", filename.c_str());
        return false;
    }

    uint32 size;
    char fourcc[5];

    while (!f.isEof())
    {
        f.read(fourcc, 4);
        f.read(&size, 4);

        flipcc(fourcc);
        fourcc[4] = 0;

        size_t nextpos = f.getPos() + size;

        if (!strcmp(fourcc, "MOHD")) //header
        {
            f.read(&nTextures, 4);
            f.read(&nGroups, 4);
            f.read(&nP, 4);
            f.read(&nLights, 4);
            f.read(&nModels, 4);
            f.read(&nDoodads, 4);
            f.read(&nDoodadSets, 4);
            f.read(&col, 4);
            f.read(&RootWMOID, 4);
            f.read(bbcorn1, 12);
            f.read(bbcorn2, 12);
            f.read(&liquidType, 4);
            break;
        }
        /*
        else if (!strcmp(fourcc,"MOTX"))
        {
        }
        else if (!strcmp(fourcc,"MOMT"))
        {
        }
        else if (!strcmp(fourcc,"MOGN"))
        {
        }
        else if (!strcmp(fourcc,"MOGI"))
        {
        }
        else if (!strcmp(fourcc,"MOLT"))
        {
        }
        else if (!strcmp(fourcc,"MODN"))
        {
        }
        else if (!strcmp(fourcc,"MODS"))
        {
        }
        else if (!strcmp(fourcc,"MODD"))
        {
        }
        else if (!strcmp(fourcc,"MOSB"))
        {
        }
        else if (!strcmp(fourcc,"MOPV"))
        {
        }
        else if (!strcmp(fourcc,"MOPT"))
        {
        }
        else if (!strcmp(fourcc,"MOPR"))
        {
        }
        else if (!strcmp(fourcc,"MFOG"))
        {
        }
        */
        f.seek((int)nextpos);
    }
    f.close();
    return true;
}

bool WMORoot::ConvertToVMAPRootWmo(FILE* pOutfile, const void *szRawVMAPMagic)
{
    //printf("Convert RootWmo...\n");

    fwrite(szRawVMAPMagic, 1, 8, pOutfile);
    unsigned int nVectors = 0;
    fwrite(&nVectors, sizeof(nVectors), 1, pOutfile); // will be filled later
    fwrite(&nGroups, 4, 1, pOutfile);
    fwrite(&RootWMOID, 4, 1, pOutfile);
    return true;
}

WMORoot::~WMORoot()
{
}

WMOGroup::WMOGroup(std::string& filename) : filename(filename),
    MOPY(0), MOVI(0), MoviEx(0), MOVT(0), MOBA(0), MobaEx(0), hlq(0), LiquEx(0), LiquBytes(0)
{
}

bool WMOGroup::open()
{
    HANDLE mpqHandle;

    if (!OpenNewestFile(filename.c_str(), &mpqHandle))
    {
        printf("Error opening WMOGroup %s\n", filename.c_str());
    }

    MPQFile f(mpqHandle, filename.c_str());
    if (f.isEof())
    {
        printf(" No such file.\n");
        return false;
    }
    uint32 size;
    char fourcc[5];
    while (!f.isEof())
    {
        f.read(fourcc, 4);
        f.read(&size, 4);
        flipcc(fourcc);
        if (!strcmp(fourcc, "MOGP")) //Fix sizeoff = Data size.
        {
            size = 68;
        }
        fourcc[4] = 0;
        size_t nextpos = f.getPos() + size;
        LiquEx_size = 0;
        liquflags = 0;

        if (!strcmp(fourcc, "MOGP")) //header
        {
            f.read(&groupName, 4);
            f.read(&descGroupName, 4);
            f.read(&mogpFlags, 4);
            f.read(bbcorn1, 12);
            f.read(bbcorn2, 12);
            f.read(&moprIdx, 2);
            f.read(&moprNItems, 2);
            f.read(&nBatchA, 2);
            f.read(&nBatchB, 2);
            f.read(&nBatchC, 4);
            f.read(&fogIdx, 4);
            f.read(&liquidType, 4);
            f.read(&groupWMOID, 4);

        }
        else if (!strcmp(fourcc, "MOPY"))
        {
            MOPY = new char[size];
            mopy_size = size;
            nTriangles = (int)size / 2;
            f.read(MOPY, size);
        }
        else if (!strcmp(fourcc, "MOVI"))
        {
            MOVI = new uint16[size / 2];
            f.read(MOVI, size);
        }
        else if (!strcmp(fourcc, "MOVT"))
        {
            MOVT = new float[size / 4];
            f.read(MOVT, size);
            nVertices = (int)size / 12;
        }
        else if (!strcmp(fourcc, "MONR"))
        {
        }
        else if (!strcmp(fourcc, "MOTV"))
        {
        }
        else if (!strcmp(fourcc, "MOBA"))
        {
            MOBA = new uint16[size / 2];
            moba_size = size / 2;
            f.read(MOBA, size);
        }
        else if (!strcmp(fourcc, "MLIQ"))
        {
            liquflags |= 1;
            hlq = new WMOLiquidHeader;
            f.read(hlq, 0x1E);
            LiquEx_size = sizeof(WMOLiquidVert) * hlq->xverts * hlq->yverts;
            LiquEx = new WMOLiquidVert[hlq->xverts * hlq->yverts];
            f.read(LiquEx, LiquEx_size);
            int nLiquBytes = hlq->xtiles * hlq->ytiles;
            LiquBytes = new char[nLiquBytes];
            f.read(LiquBytes, nLiquBytes);

            /* std::ofstream llog("Buildings/liquid.log", ios_base::out | ios_base::app);
            llog << filename;
            llog << "\nbbox: " << bbcorn1[0] << ", " << bbcorn1[1] << ", " << bbcorn1[2] << " | " << bbcorn2[0] << ", " << bbcorn2[1] << ", " << bbcorn2[2];
            llog << "\nlpos: " << hlq->pos_x << ", " << hlq->pos_y << ", " << hlq->pos_z;
            llog << "\nx-/yvert: " << hlq->xverts << "/" << hlq->yverts << " size: " << size << " expected size: " << 30 + hlq->xverts*hlq->yverts*8 + hlq->xtiles*hlq->ytiles << std::endl;
            llog.close(); */
        }
        f.seek((int)nextpos);
    }
    f.close();
    return true;
}

int WMOGroup::ConvertToVMAPGroupWmo(FILE* output, WMORoot* rootWMO, bool pPreciseVectorData, int iCoreNumber)
{
    fwrite(&mogpFlags, sizeof(uint32), 1, output);
    fwrite(&groupWMOID, sizeof(uint32), 1, output);
    // group bound
    fwrite(bbcorn1, sizeof(float), 3, output);
    fwrite(bbcorn2, sizeof(float), 3, output);
    fwrite(&liquflags, sizeof(uint32), 1, output);
    int nColTriangles = 0;
    if (pPreciseVectorData)
    {
        char GRP[] = "GRP ";
        fwrite(GRP, 1, 4, output);

        int k = 0;
        int moba_batch = moba_size / 12;
        MobaEx = new int[moba_batch * 4];
        for (int i = 8; i < moba_size; i += 12)
        {
            MobaEx[k++] = MOBA[i];
        }
        int moba_size_grp = moba_batch * 4 + 4;
        fwrite(&moba_size_grp, 4, 1, output);
        fwrite(&moba_batch, 4, 1, output);
        fwrite(MobaEx, 4, k, output);
        delete [] MobaEx;

        uint32 nIdexes = nTriangles * 3;

        if (fwrite("INDX", 4, 1, output) != 1)
        {
            printf("Error while writing file nbraches ID");
            exit(0);
        }
        int wsize = sizeof(uint32) + sizeof(unsigned short) * nIdexes;
        if (fwrite(&wsize, sizeof(int), 1, output) != 1)
        {
            printf("Error while writing file wsize");
            exit(0);
        }
        if (fwrite(&nIdexes, sizeof(uint32), 1, output) != 1)
        {
            printf("Error while writing file nIndexes");
            exit(0);
        }
        if (nIdexes > 0)
        {
            if (fwrite(MOVI, sizeof(unsigned short), nIdexes, output) != nIdexes)
            {
                printf("Error while writing file indexarray");
                exit(0);
            }
        }

        if (fwrite("VERT", 4, 1, output) != 1)
        {
            printf("Error while writing file nbraches ID");
            exit(0);
        }
        wsize = sizeof(int) + sizeof(float) * 3 * nVertices;
        if (fwrite(&wsize, sizeof(int), 1, output) != 1)
        {
            printf("Error while writing file wsize");
            exit(0);
        }
        if (fwrite(&nVertices, sizeof(int), 1, output) != 1)
        {
            printf("Error while writing file nVertices");
            exit(0);
        }
        if (nVertices > 0)
        {
            if (fwrite(MOVT, sizeof(float) * 3, nVertices, output) != nVertices)
            {
                printf("Error while writing file vectors");
                exit(0);
            }
        }

        nColTriangles = nTriangles;
    }
    else
    {
        char GRP[] = "GRP ";
        fwrite(GRP, 1, 4, output);
        int k = 0;
        int moba_batch = moba_size / 12;
        MobaEx = new int[moba_batch * 4];
        for (int i = 8; i < moba_size; i += 12)
        {
            MobaEx[k++] = MOBA[i];
        }

        int moba_size_grp = moba_batch * 4 + 4;
        fwrite(&moba_size_grp, 4, 1, output);
        fwrite(&moba_batch, 4, 1, output);
        fwrite(MobaEx, 4, k, output);
        delete [] MobaEx;

        //-------INDX------------------------------------
        //-------MOPY--------
        MoviEx = new uint16[nTriangles * 3]; // "worst case" size...
        int* IndexRenum = new int[nVertices];
        memset(IndexRenum, 0xFF, nVertices * sizeof(int));
        for (int i = 0; i < nTriangles; ++i)
        {
            // Skip no collision triangles
            if (MOPY[2 * i]&WMO_MATERIAL_NO_COLLISION ||
                !(MOPY[2 * i] & (WMO_MATERIAL_HINT | WMO_MATERIAL_COLLIDE_HIT)))
            {
                continue;
            }
            // Use this triangle
            for (int j = 0; j < 3; ++j)
            {
                IndexRenum[MOVI[3 * i + j]] = 1;
                MoviEx[3 * nColTriangles + j] = MOVI[3 * i + j];
            }
            ++nColTriangles;
        }

        // assign new vertex index numbers
        int nColVertices = 0;
        for (uint32 i = 0; i < nVertices; ++i)
        {
            if (IndexRenum[i] == 1)
            {
                IndexRenum[i] = nColVertices;
                ++nColVertices;
            }
        }

        // translate triangle indices to new numbers
        for (int i = 0; i < 3 * nColTriangles; ++i)
        {
            assert(MoviEx[i] < nVertices);
            MoviEx[i] = IndexRenum[MoviEx[i]];
        }

        // write triangle indices
        int INDX[] = {0x58444E49, nColTriangles * 6 + 4, nColTriangles * 3};
        fwrite(INDX, 4, 3, output);
        fwrite(MoviEx, 2, nColTriangles * 3, output);

        // write vertices
        int VERT[] = {0x54524556, static_cast<int>(nColVertices * 3 * sizeof(float) + 4), nColVertices}; // "VERT"
        int check = 3 * nColVertices;
        fwrite(VERT, 4, 3, output);
        for (uint32 i = 0; i < nVertices; ++i)
            if (IndexRenum[i] >= 0)
            {
                check -= fwrite(MOVT + 3 * i, sizeof(float), 3, output);
            }

        assert(check == 0);

        delete [] MoviEx;
        delete [] IndexRenum;
    }

    //------LIQU------------------------
    if (LiquEx_size != 0)
    {
        int LIQU_h[] = {0x5551494C, static_cast<int>(sizeof(WMOLiquidHeader) + LiquEx_size + hlq->xtiles* hlq->ytiles)}; // "LIQU"
        fwrite(LIQU_h, 4, 2, output);

        // according to WoW.Dev Wiki:
        uint32 liquidEntry;
        if (rootWMO->liquidType & 4)
        {
            liquidEntry = liquidType;
        }
        else if (liquidType == 15)
        {
            liquidEntry = 0;
        }
        else
        {
            liquidEntry = liquidType + 1;
        }

        if (!liquidEntry)
        {
            int v1; // edx@1
            int v2; // eax@1

            v1 = hlq->xtiles * hlq->ytiles;
            v2 = 0;
            if (v1 > 0)
            {
                while ((LiquBytes[v2] & 0xF) == 15)
                {
                    ++v2;
                    if (v2 >= v1)
                    {
                        break;
                    }
                }

                if (v2 < v1 && (LiquBytes[v2] & 0xF) != 15)
                {
                    liquidEntry = (LiquBytes[v2] & 0xF) + 1;
                }
            }
        }

        if (liquidEntry && liquidEntry < 21)
        {
            switch (((uint8)liquidEntry - 1) & 3)
            {
                case 0:
                    if (iCoreNumber == CLIENT_WOTLK || iCoreNumber == CLIENT_CATA)
                    {
                        liquidEntry = ((mogpFlags & 0x80000) != 0) + 13;
                    }
                    if (iCoreNumber == CLIENT_CLASSIC || iCoreNumber == CLIENT_TBC)
                    {
                        liquidEntry = ((mogpFlags & 0x80000) != 0) + 1;
                        if (iCoreNumber == CLIENT_TBC)
                        {
                            if (liquidEntry == 1)   // water type
                            {
                                if (filename.find("coilfang_raid") != std::string::npos)
                                {
                                    // set water type to special coilfang raid water
                                    liquidEntry = 41;
                                }
                            }
                        }
                    }
                    break;
                case 1:
                    if (iCoreNumber == CLIENT_CLASSIC || iCoreNumber == CLIENT_TBC)
                    {
                        liquidEntry = 2;        // ocean
                    }
                    if (iCoreNumber == CLIENT_WOTLK || iCoreNumber == CLIENT_CATA)
                    {
                        liquidEntry = 14;
                    }
                    break;
                case 2:
                    if (iCoreNumber == CLIENT_CLASSIC || iCoreNumber == CLIENT_TBC)
                    {
                    liquidEntry = 3;        // magma
                    }
                    if (iCoreNumber == CLIENT_WOTLK || iCoreNumber == CLIENT_CATA)
                    {
                        liquidEntry = 19;
                    }
                    break;
                case 3:
                    if (iCoreNumber == CLIENT_CLASSIC || iCoreNumber == CLIENT_TBC)
                    {
                        if ((filename.find("stratholme_raid") != std::string::npos) || (filename.find("Stratholme_raid") != std::string::npos))
                        {
                            liquidEntry = 21;   // Naxxramas slime
                        }
                        else
                        {
                            liquidEntry = 4;
                        }    // Normal slime
                    }
                    if (iCoreNumber == CLIENT_WOTLK || iCoreNumber == CLIENT_CATA)
                    {
                        liquidEntry = 20;
                    }
                    break;
                default:
                    break;
            }
        }

        hlq->type = liquidEntry;

        /* std::ofstream llog("Buildings/liquid.log", ios_base::out | ios_base::app);
        llog << filename;
        llog << ":\nliquidEntry: " << liquidEntry << " type: " << hlq->type << " (root:" << rootWMO->liquidType << " group:" << liquidType << ")\n";
        llog.close(); */

        fwrite(hlq, sizeof(WMOLiquidHeader), 1, output);
        // only need height values, the other values are unknown anyway
        for (uint32 i = 0; i < LiquEx_size / sizeof(WMOLiquidVert); ++i)
        {
            fwrite(&LiquEx[i].height, sizeof(float), 1, output);
        }
        // todo: compress to bit field
        fwrite(LiquBytes, 1, hlq->xtiles * hlq->ytiles, output);
    }

    return nColTriangles;
}

WMOGroup::~WMOGroup()
{
    delete [] MOPY;
    delete [] MOVI;
    delete [] MOVT;
    delete [] MOBA;
    delete hlq;
    delete [] LiquEx;
    delete [] LiquBytes;
}

//WmoInstName is in the form MD5/name.wmo
WMOInstance::WMOInstance(MPQFile& f, std::string& WmoInstName, uint32 mapID, uint32 tileX, uint32 tileY, FILE* pDirfile)
{
    pos = Vec3D(0, 0, 0);

    float ff[3];
    f.read(&id, 4);
    f.read(ff, 12);
    pos = Vec3D(ff[0], ff[1], ff[2]);
    f.read(ff, 12);
    rot = Vec3D(ff[0], ff[1], ff[2]);
    f.read(ff, 12);
    pos2 = Vec3D(ff[0], ff[1], ff[2]);
    f.read(ff, 12);
    pos3 = Vec3D(ff[0], ff[1], ff[2]);
    f.read(&d2, 4);

    uint16 trash, adtId;
    f.read(&adtId, 2);
    f.read(&trash, 2);

    //-----------add_in _dir_file----------------

    char tempname[512];
    sprintf(tempname, "%s/%s", szWorkDirWmo, WmoInstName.c_str());
    FILE* input;
    input = fopen(tempname, "r+b");

    if (!input)
    {
        printf("WMOInstance::WMOInstance: couldn't open %s\n", tempname);
        return;
    }

    fseek(input, 8, SEEK_SET); // get the correct no of vertices
    int nVertices;
    size_t file_read = fread(&nVertices, sizeof(int), 1, input);
    fclose(input);

    if (nVertices == 0 || file_read <= 0)
    {
        return;
    }

    float x, z;
    x = pos.x;
    z = pos.z;
    if (x == 0 && z == 0)
    {
        pos.x = 533.33333f * 32;
        pos.z = 533.33333f * 32;
    }
    pos = fixCoords(pos);
    pos2 = fixCoords(pos2);
    pos3 = fixCoords(pos3);

    float scale = 1.0f;
    uint32 flags = MOD_HAS_BOUND;
    if (tileX == 65 && tileY == 65)
    {
        flags |= MOD_WORLDSPAWN;
    }
    //write mapID, tileX, tileY, Flags, ID, Pos, Rot, Scale, Bound_lo, Bound_hi, name
    fwrite(&mapID, sizeof(uint32), 1, pDirfile);
    fwrite(&tileX, sizeof(uint32), 1, pDirfile);
    fwrite(&tileY, sizeof(uint32), 1, pDirfile);
    fwrite(&flags, sizeof(uint32), 1, pDirfile);
    fwrite(&adtId, sizeof(uint16), 1, pDirfile);
    fwrite(&id, sizeof(uint32), 1, pDirfile);
    fwrite(&pos, sizeof(float), 3, pDirfile);
    fwrite(&rot, sizeof(float), 3, pDirfile);
    fwrite(&scale, sizeof(float), 1, pDirfile);
    fwrite(&pos2, sizeof(float), 3, pDirfile);
    fwrite(&pos3, sizeof(float), 3, pDirfile);
    uint32 nlen = WmoInstName.length();
    fwrite(&nlen, sizeof(uint32), 1, pDirfile);
    fwrite(WmoInstName.c_str(), sizeof(char), nlen, pDirfile);

}

bool ExtractSingleWmo(std::string& fname, int iCoreNumber, const void *szRawVMAPMagic)
{
    // Copy files from archive
    char szLocalFile[1024];
    std::string plain_name = GetUniformName(fname);

    sprintf(szLocalFile, "%s/%s", szWorkDirWmo, plain_name.c_str());


    if (FileExists(szLocalFile))
    {
        return true;
    }

    int p = 0;
    //Select root wmo files
    const char* rchr = strrchr(plain_name.c_str(), '_');
    if (rchr != NULL)
    {
        char cpy[4];
        strncpy((char*)cpy, rchr, 4);
        for (int i = 0; i < 4; ++i)
        {
            int m = cpy[i];
            if (isdigit(m))
            {
                p++;
            }
        }
    }

    if (p == 3)
    {
        return true;
    }

    bool file_ok = true;
    printf(" Extracting %s\n", fname.c_str());

    HANDLE wmoHandle;

    if (!OpenNewestFile(fname.c_str(), &wmoHandle))
    {
        printf("Error opening WMO file %s\n", fname.c_str());
    }

    WMORoot froot(fname);
    if (!froot.open())
    {
        printf("Couldn't open RootWmo!!!\n");
        return true;
    }

    FILE* output = fopen(szLocalFile, "wb");
    if (!output)
    {
        printf("Couldn't open %s for writing!\n", szLocalFile);
        return false;
    }

    froot.ConvertToVMAPRootWmo(output,szRawVMAPMagic);
    int Wmo_nVertices = 0;
    if (froot.nGroups != 0)
    {
        for (uint32 i = 0; i < froot.nGroups; ++i)
        {
            char temp[1024];
            strcpy(temp, fname.c_str());
            temp[fname.length() - 4] = 0;
            char groupFileName[1024];
            sprintf(groupFileName, "%s_%03d.wmo", temp, i);

            std::string s(groupFileName);

            WMOGroup fgroup(s);
            if (!fgroup.open())
            {
                printf("Could not open all Group file for: %s\n", plain_name.c_str());
                file_ok = false;
                break;
            }

            Wmo_nVertices += fgroup.ConvertToVMAPGroupWmo(output, &froot, preciseVectorData, iCoreNumber);
        }
    }

    fseek(output, 8, SEEK_SET); // store the correct no of vertices
    fwrite(&Wmo_nVertices, sizeof(int), 1, output);
    fclose(output);

    // Delete the extracted file in the case of an error
    if (!file_ok)
    {
        remove(szLocalFile);
    }
    return true;
}

bool ExtractWmo(int iCoreNumber, const void *szRawVMAPMagic)
{
    bool success = true;

    // Extract WMO in the reverse-order of the MPQ priority (ensure the highest priority WMO is erasing others)

    for (ArchiveSet::reverse_iterator ar_itr = gOpenArchives.rbegin(); ar_itr != gOpenArchives.rend() && success; ++ar_itr)
    {
        SFILE_FIND_DATA data;
        HANDLE find = SFileFindFirstFile(*ar_itr, "*.wmo", &data, NULL);
        if (find != NULL)
        {
            do
            {
                std::string str = data.cFileName;
                printf("Extracting wmo %s\n", str.c_str());
                success |= ExtractSingleWmo(str, iCoreNumber, szRawVMAPMagic);
            } while (SFileFindNextFile(find, &data));
        }
        SFileFindClose(find);
    }

    if (success)
    {
        printf("\n Extraction of WMO's complete, No fatal errors\n");
    }
    printf("\n Reading Maps\n");
    printf(" _______________________________________________________\n");

    return success;
}
