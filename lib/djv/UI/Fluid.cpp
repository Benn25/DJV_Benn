// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the DJV project.

#include <djv/UI/Fluid.h>

#if defined(FTK_SDL3)
#include <SDL3/SDL.h>
#endif // FTK_SDL3

#include <cmath>

namespace djv
{
    namespace ui
    {
        int getScreenWidth()
        {
            int out = 0;
#if defined(FTK_SDL3)
            int count = 0;
            if (SDL_Window** windows = SDL_GetWindows(&count))
            {
                // The main window is the first made.
                if (count > 0)
                {
                    if (const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(
                        SDL_GetDisplayForWindow(windows[0])))
                    {
                        out = static_cast<int>(std::lround(
                            mode->w * (mode->pixel_density > 0.F ? mode->pixel_density : 1.F)));
                    }
                }
                SDL_free(windows);
            }
#endif // FTK_SDL3
            return out;
        }

        int getFluidMinWidth(float displayScale)
        {
            const int screenWidth = getScreenWidth();
            return screenWidth > 0 ?
                static_cast<int>(std::lround(screenWidth * .05F)) :
                static_cast<int>(std::lround(100 * displayScale));
        }
    }
}
