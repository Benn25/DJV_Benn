// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the DJV project.

#pragma once

#include <djv/UI/Export.h>

#include <ftk/UI/IWidget.h>

namespace djv
{
    namespace ui
    {
        //! A text that wraps onto as many lines as the width it is given
        //! needs, rather than setting how wide its panel has to be: a panel
        //! narrowed grows taller. Under the fluid minimum width it stops
        //! wrapping and is cut. Lines are broken between words; a line break
        //! in the text is kept, and a word longer than the width is left
        //! whole.
        class DJV_UI_API_TYPE WrapLabel : public ftk::IWidget
        {
            FTK_NON_COPYABLE(WrapLabel);

        protected:
            void _init(
                const std::shared_ptr<ftk::Context>&,
                const std::string& text,
                const std::shared_ptr<IWidget>& parent);

            WrapLabel();

        public:
            DJV_UI_API virtual ~WrapLabel();

            //! Create a new widget.
            DJV_UI_API static std::shared_ptr<WrapLabel> create(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Create a new widget.
            DJV_UI_API static std::shared_ptr<WrapLabel> create(
                const std::shared_ptr<ftk::Context>&,
                const std::string& text,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get the text.
            DJV_UI_API const std::string& getText() const;

            //! Set the text.
            DJV_UI_API void setText(const std::string&);

            //! Set the margin around the text.
            DJV_UI_API void setMarginRole(ftk::SizeRole);

            DJV_UI_API ftk::Size2I getSizeHint() const override;
            DJV_UI_API void setGeometry(const ftk::Box2I&) override;
            DJV_UI_API void sizeHintEvent(const ftk::SizeHintEvent&) override;
            DJV_UI_API void drawEvent(const ftk::Box2I&, const ftk::DrawEvent&) override;

        private:
            FTK_PRIVATE();
        };
    }
}
