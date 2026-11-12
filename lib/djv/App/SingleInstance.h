// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the DJV project.

#pragma once

#include <djv/App/Export.h>

#include <memory>
#include <string>
#include <vector>

namespace djv
{
    namespace app
    {
        //! Files opened from Windows -- a double click, "Open with" -- go to
        //! the DJV already running, as tabs, rather than to an instance of
        //! their own: each instance fills a cache as large as the settings
        //! allow, and two of them took the memory of both. A run with no
        //! files, as the taskbar's middle click starts, or with any option
        //! on its command line is an instance of its own, as before.
        //!
        //! Hand the files named on the command line to the instance already
        //! running, the one used last. True when they were handed over and
        //! this run should end. While another instance is starting it waits
        //! for it, so that files opened together go to the one instance.
        //! Does nothing on other platforms.
        DJV_APP_API bool forwardToRunningInstance(const std::vector<std::string>& argv);

        //! Takes the files other runs hand over; there is one, in each
        //! instance with a main window. Made on the main thread, whose
        //! message loop delivers them.
        class InstanceReceiver
        {
        public:
            InstanceReceiver();
            ~InstanceReceiver();

            //! The files handed over since the last call.
            std::vector<std::string> takePaths();

            //! Bring the main window to the front, restored if minimized.
            void raise();

        private:
            struct Private;
            std::unique_ptr<Private> _p;
        };
    }
}
