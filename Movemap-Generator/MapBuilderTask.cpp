/**
 * MaNGOS is a full featured server for World of Warcraft, supporting
 * the following clients: 1.12.x, 2.4.3, 3.3.5a, 4.3.4a and 5.4.8
 *
 * Copyright (C) 2005-2015  MaNGOS project <http://getmangos.eu>
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

#include "MapBuilderTask.h"
#include "DelayExecutor.h"
#include "MapBuilder.h"

#include <ace/Guard_T.h>
#include <ace/Method_Request.h>

namespace MMAP
{
    class MapBuilderRequest : public ACE_Method_Request
    {
        private:

            MapBuilder&      m_builder;
            MapBuilderTask&  m_task;
            ACE_UINT32       m_mapId;
            char const*      m_magic;

        public:

            MapBuilderRequest(MapBuilder& m, ACE_UINT32 mapID, MapBuilderTask& t, char const* MAGIC)
                : m_builder(m), m_task(t), m_mapId(mapID), m_magic(MAGIC)
            {
            }

            virtual int call()
            {
                m_builder.buildMap(m_mapId, m_magic);
                m_task.build_finished();
                return 0;
            }
    };

    MapBuilderTask::MapBuilderTask():m_executor(), m_mutex(), m_condition(m_mutex), pending_requests(0)
    {
    }

    MapBuilderTask::~MapBuilderTask()
    {
        deactivate();
    }

    int MapBuilderTask::activate(int num_threads)
    {
        return m_executor.activate(num_threads);
    }

    int MapBuilderTask::deactivate()
    {
        wait();

        return m_executor.deactivate();
    }

    int MapBuilderTask::wait()
    {
        ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, m_mutex, -1);

        while (pending_requests > 0)
            m_condition.wait();

        return 0;
    }

    int MapBuilderTask::schedule_build(MapBuilder& mb, ACE_UINT32 mapID, char const* magic)
    {
        ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, m_mutex, -1);

        ++pending_requests;

        if (m_executor.execute(new MapBuilderRequest(mb, mapID, *this, magic)) == -1)
        {
            ACE_DEBUG((LM_ERROR, ACE_TEXT("(%t) \n"), ACE_TEXT("Failed to schedule map build")));

            --pending_requests;
            return -1;
        }

        return 0;
    }

    bool MapBuilderTask::activated()
    {
        return m_executor.activated();
    }

    void MapBuilderTask::build_finished()
    {
        ACE_GUARD(ACE_Thread_Mutex, guard, m_mutex);

        if (pending_requests == 0)
        {
            ACE_ERROR((LM_ERROR, ACE_TEXT("(%t)\n"), ACE_TEXT("MapBuilderTask::build_finished BUG, report to developers")));
            return;
        }

        --pending_requests;

        m_condition.broadcast();
    }
}