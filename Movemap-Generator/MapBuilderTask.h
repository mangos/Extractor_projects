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

#ifndef _MAP_BUILDER_TASK_H_INCLUDED
#define _MAP_BUILDER_TASK_H_INCLUDED

#include <ace/Thread_Mutex.h>
#include <ace/Condition_Thread_Mutex.h>

#include "DelayExecutor.h"

namespace MMAP
{
    class MapBuilder;

    class MapBuilderTask
    {
        public:
            MapBuilderTask();
            virtual ~MapBuilderTask();

            int schedule_build(MapBuilder& mb, ACE_UINT32 mapId, char const* MAGIC);
            void build_finished();

            int wait();

            int activate(int num_threads);

            int deactivate();

            bool activated();

        private:
            DelayExecutor m_executor;
            ACE_Thread_Mutex m_mutex;
            ACE_Condition_Thread_Mutex m_condition;
            size_t pending_requests;

    };
}
#endif //_MAP_BUILDER_TASK_H_INCLUDED