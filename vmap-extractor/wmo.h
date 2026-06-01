/**
 * MaNGOS is a full featured server for World of Warcraft, supporting
 * the following clients: 1.12.x, 2.4.3, 3.3.5a, 4.3.4a and 5.4.8
 *
 * Copyright (C) 2005-2026 MaNGOS <https://www.getmangos.eu>
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

#ifndef WMO_H
#define WMO_H

#define TILESIZE (533.33333f)
#define CHUNKSIZE ((TILESIZE) / 16.0f)

#include <string>
#include <set>
#include "vec3d.h"
#include <mpq.h>
#include <loadlib.h>

// MOPY flags
#define WMO_MATERIAL_NOCAMCOLLIDE    0x01
#define WMO_MATERIAL_DETAIL          0x02
#define WMO_MATERIAL_NO_COLLISION    0x04
#define WMO_MATERIAL_HINT            0x08
#define WMO_MATERIAL_RENDER          0x10
#define WMO_MATERIAL_COLLIDE_HIT     0x20
#define WMO_MATERIAL_WALL_SURFACE    0x40

/**
 * @brief
 *
 */
class WMORoot
{
    public:
        uint32 nTextures;      /**< Number of textures in WMO */
        uint32 nGroups;        /**< Number of groups in WMO */
        uint32 nP;             /**< Number of portals in WMO */
        uint32 nLights;        /**< Number of lights in WMO */
        uint32 nModels;        /**< Number of models in WMO */
        uint32 nDoodads;       /**< Number of doodads in WMO */
        uint32 nDoodadSets;    /**< Number of doodad sets in WMO */
        uint32 RootWMOID;      /**< Root WMO identifier */
        uint32 liquidType;     /**< Liquid type for WMO */
        unsigned int col;      /**< Color/ambient information */
        float bbcorn1[3];      /**< Bounding box corner 1 (x,y,z) */
        float bbcorn2[3];      /**< Bounding box corner 2 (x,y,z) */

        /**
         * @brief
         *
         * @param filename
         */
        WMORoot(std::string& filename);

        /**
         * @brief
         *
         */
        ~WMORoot();

        /**
         * @brief
         *
         * @return bool
         */
        bool open();

        /**
         * @brief
         *
         * @param output
         * @return bool
         */
        bool ConvertToVMAPRootWmo(FILE* output, const void *szRawVMAPMagic);
    private:
        std::string filename; /**< WMO filename for this group */
};

/**
 * @brief
 *
 */
struct WMOLiquidHeader
{
    int xverts;           /**< Number of vertices in X direction */
    int yverts;           /**< Number of vertices in Y direction */
    int xtiles;           /**< Number of tiles in X direction */
    int ytiles;           /**< Number of tiles in Y direction */
    float pos_x;          /**< Liquid position X coordinate */
    float pos_y;          /**< Liquid position Y coordinate */
    float pos_z;          /**< Liquid position Z coordinate */
    short type;           /**< Liquid type identifier */
};

/**
 * @brief
 *
 */
struct WMOLiquidVert
{
    uint16 unk1;         /**< Unknown field 1 - reserved for future use */
    uint16 unk2;         /**< Unknown field 2 - reserved for future use */
    float height;        /**< Height value for liquid vertex */
};

/**
 * @brief
 *
 */
class WMOGroup
{
    public:
        // MOGP
        int groupName;              /**< Group name identifier */
        int descGroupName;          /**< Description group name identifier */
        int mogpFlags;              /**< MOGP flags for group properties */
        float bbcorn1[3];           /**< Bounding box corner 1 (x,y,z) */
        float bbcorn2[3];           /**< Bounding box corner 2 (x,y,z) */
        uint16 moprIdx;             /**< MOPR index */
        uint16 moprNItems;          /**< Number of MOPR items */
        uint16 nBatchA;             /**< Number of batch A items */
        uint16 nBatchB;             /**< Number of batch B items */
        uint32 nBatchC;             /**< Number of batch C items */
        uint32 fogIdx;              /**< Fog index */
        uint32 liquidType;          /**< Liquid type for group */
        uint32 groupWMOID;          /**< Group WMO identifier */

        int mopy_size;              /**< Size of MOPY data */
        int moba_size;              /**< Size of MOBA data */
        int LiquEx_size;            /**< Size of liquid data */
        unsigned int nVertices;     /**< Number of vertices when loaded */
        int nTriangles;             /**< Number of triangles when loaded */
        char* MOPY;                 /**< Material/operation data */
        uint16* MOVI;               /**< Vertex indices */
        uint16* MoviEx;             /**< Extended vertex indices */
        float* MOVT;                /**< Vertex positions */
        uint16* MOBA;               /**< Batches */
        int* MobaEx;                /**< Extended batch data */
        WMOLiquidHeader* hlq;       /**< Liquid header */
        WMOLiquidVert* LiquEx;      /**< Liquid vertices */
        char* LiquBytes;            /**< Liquid byte data */
        uint32 liquflags;           /**< Liquid flags */

        /**
         * @brief
         *
         * @param filename
         */
        WMOGroup(std::string& filename);

        /**
         * @brief
         *
         */
        ~WMOGroup();

        /**
         * @brief
         *
         * @return bool
         */
        bool open();

        /**
         * @brief
         *
         * @param output
         * @param rootWMO
         * @param pPreciseVectorData
         * @return int
         */
        int ConvertToVMAPGroupWmo(FILE* output, WMORoot* rootWMO, bool pPreciseVectorData, int iCoreNumber);

    private:
        std::string filename; /**< WMO filename for this group */
};

/**
 * @brief
 *
 */
class WMOInstance
{
    static std::set<int> ids; /**< TODO */

    public:
        std::string MapName; /**< TODO */
        int currx; /**< TODO */
        int curry; /**< TODO */
        WMOGroup* wmo; /**< TODO */
        Vec3D pos; /**< TODO */
        Vec3D pos2, pos3, rot; /**< TODO */
        uint32 indx, id, d2, d3; /**< TODO */
        int doodadset; /**< TODO */

        /**
         * @brief
         *
         * @param f
         * @param WmoInstName
         * @param mapID
         * @param tileX
         * @param tileY
         * @param pDirfile
         */
        WMOInstance(MPQFile& f, std::string& WmoInstName, uint32 mapID, uint32 tileX, uint32 tileY, FILE* pDirfile);

        /**
         * @brief
         *
         */
        static void reset();
};

/**
 * @brief
 *
 * @param fname
 * @return bool
 */
bool ExtractSingleWmo(std::string& fname, int iCoreNumber, const void *szRawVMAPMagic);

/**
 * @brief
 *
 * @param
 * @return bool
 */
bool ExtractWmo(int iCoreNumber, const void *szRawVMAPMagic);

#endif
