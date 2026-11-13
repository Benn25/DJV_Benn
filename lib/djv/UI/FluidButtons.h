// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the DJV project.

#pragma once

#include <djv/UI/Export.h>

#include <ftk/UI/ComboBox.h>
#include <ftk/UI/PushButton.h>

namespace djv
{
    namespace ui
    {
        //! A push button for a panel, fluid as Blender's are: buttons share
        //! the row's width, and one narrower than its text cuts the text
        //! short with an ellipsis, down to its icon.
        class DJV_UI_API_TYPE PushButton : public ftk::PushButton
        {
            FTK_NON_COPYABLE(PushButton);

        protected:
            void _init(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<IWidget>& parent);

            PushButton();

        public:
            DJV_UI_API virtual ~PushButton();

            //! Create a new widget.
            DJV_UI_API static std::shared_ptr<PushButton> create(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Create a new widget.
            DJV_UI_API static std::shared_ptr<PushButton> create(
                const std::shared_ptr<ftk::Context>&,
                const std::string& text,
                const std::shared_ptr<IWidget>& parent = nullptr);

            DJV_UI_API ftk::Size2I getSizeHint() const override;
            DJV_UI_API void sizeHintEvent(const ftk::SizeHintEvent&) override;
            DJV_UI_API void drawEvent(const ftk::Box2I&, const ftk::DrawEvent&) override;

        private:
            int _minWidth = 0;
            int _margin = 0;
            int _pad = 0;
            int _keyFocus = 0;
            int _cornerRadius = 0;
            ftk::FontInfo _fontInfo;
            ftk::FontMetrics _fontMetrics;
        };

        //! A combo box for a panel, fluid: narrowed, the current item is cut
        //! short with an ellipsis rather than setting how wide the panel has
        //! to be. The menu shows the items whole.
        class DJV_UI_API_TYPE ComboBox : public ftk::ComboBox
        {
            FTK_NON_COPYABLE(ComboBox);

        protected:
            void _init(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<IWidget>& parent);

            ComboBox();

        public:
            DJV_UI_API virtual ~ComboBox();

            //! Create a new widget.
            DJV_UI_API static std::shared_ptr<ComboBox> create(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Create a new widget.
            DJV_UI_API static std::shared_ptr<ComboBox> create(
                const std::shared_ptr<ftk::Context>&,
                const std::vector<ftk::ComboBoxItem>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Create a new widget.
            DJV_UI_API static std::shared_ptr<ComboBox> create(
                const std::shared_ptr<ftk::Context>&,
                const std::vector<std::string>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            DJV_UI_API ftk::Size2I getSizeHint() const override;
            DJV_UI_API void setGeometry(const ftk::Box2I&) override;
            DJV_UI_API void sizeHintEvent(const ftk::SizeHintEvent&) override;

        private:
            void _elideUpdate();

            int _width = 0;
            int _chrome = 0;
            int _minWidth = 0;
            ftk::FontInfo _fontInfo;
        };
    }
}
