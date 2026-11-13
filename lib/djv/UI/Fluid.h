// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the DJV project.

#pragma once

#include <djv/UI/Export.h>

namespace djv
{
    namespace ui
    {
        //! The panels' contents are fluid, as Blender's are: narrowed, their
        //! buttons and combo boxes shrink and cut their text short, and long
        //! texts wrap onto more lines. Under a part of the screen's width
        //! they stop fitting and are cut, as there is no sense in fitting
        //! them any further.

        //! The width of the screen the main window is on, in pixels; zero
        //! when it cannot be found.
        DJV_UI_API int getScreenWidth();

        //! The narrowest the fluid contents of a panel go: 5% of the screen,
        //! or a fixed width when the screen cannot be found.
        DJV_UI_API int getFluidMinWidth(float displayScale);
    }
}
