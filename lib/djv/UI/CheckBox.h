// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the DJV project.

#pragma once

#include <djv/UI/Export.h>

#include <ftk/UI/CheckBox.h>

namespace djv
{
    namespace ui
    {
        //! A check box that toggles as it is pressed, and then, while the
        //! button is held, sets the other check boxes the pointer is dragged
        //! across the same way, as Blender's do: a row of them ticked or
        //! cleared in one stroke.
        class DJV_UI_API_TYPE CheckBox : public ftk::CheckBox
        {
            FTK_NON_COPYABLE(CheckBox);

        protected:
            void _init(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<IWidget>& parent);

            CheckBox();

        public:
            DJV_UI_API virtual ~CheckBox();

            //! Create a new widget.
            DJV_UI_API static std::shared_ptr<CheckBox> create(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Create a new widget.
            DJV_UI_API static std::shared_ptr<CheckBox> create(
                const std::shared_ptr<ftk::Context>&,
                const std::string& text,
                const std::shared_ptr<IWidget>& parent = nullptr);

            DJV_UI_API void mouseMoveEvent(ftk::MouseMoveEvent&) override;
            DJV_UI_API void mousePressEvent(ftk::MouseClickEvent&) override;
            DJV_UI_API void mouseReleaseEvent(ftk::MouseClickEvent&) override;

        private:
            bool _sweep = false;
            bool _sweepChecked = false;
        };
    }
}
