// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the DJV project.

#pragma once

#include <djv/UI/Export.h>

#include <ftk/UI/IWidget.h>

#include <functional>

namespace djv
{
    namespace ui
    {
        //! A collapsible section inside a panel, drawn as Blender draws its
        //! subpanels: inset from the panel's sides, on a box of its own, with
        //! a header lower and dimmer than a panel's. ftk::Bellows was drawn
        //! as wide and as strong as the panels themselves, and its sections
        //! read as panels. The same interface as ftk::Bellows.
        //!
        //! A press on the header opens or closes it, and dragging on across
        //! the headers of the others of the same panel does the same to
        //! them, as Blender's do.
        class DJV_UI_API_TYPE SubPanel : public ftk::IWidget
        {
            FTK_NON_COPYABLE(SubPanel);

        protected:
            void _init(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<IWidget>& parent);

            SubPanel();

        public:
            DJV_UI_API virtual ~SubPanel();

            //! Create a new widget.
            DJV_UI_API static std::shared_ptr<SubPanel> create(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Create a new widget.
            DJV_UI_API static std::shared_ptr<SubPanel> create(
                const std::shared_ptr<ftk::Context>&,
                const std::string& text,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get the text.
            DJV_UI_API const std::string& getText() const;

            //! Set the text.
            DJV_UI_API void setText(const std::string&);

            //! Get the widget.
            DJV_UI_API const std::shared_ptr<IWidget>& getWidget() const;

            //! Set the widget.
            DJV_UI_API void setWidget(const std::shared_ptr<IWidget>&);

            //! Get whether it is open.
            DJV_UI_API bool isOpen() const;

            //! Set whether it is open.
            DJV_UI_API void setOpen(bool);

            //! Set the open callback.
            DJV_UI_API void setOpenCallback(const std::function<void(bool)>&);

            //! Get the tool widget, at the right of the header.
            DJV_UI_API const std::shared_ptr<IWidget>& getToolWidget() const;

            //! Set the tool widget.
            DJV_UI_API void setToolWidget(const std::shared_ptr<IWidget>&);

            DJV_UI_API ftk::Size2I getSizeHint() const override;
            DJV_UI_API void setGeometry(const ftk::Box2I&) override;
            DJV_UI_API void sizeHintEvent(const ftk::SizeHintEvent&) override;
            DJV_UI_API void drawEvent(const ftk::Box2I&, const ftk::DrawEvent&) override;
            DJV_UI_API void mouseMoveEvent(ftk::MouseMoveEvent&) override;
            DJV_UI_API void mousePressEvent(ftk::MouseClickEvent&) override;
            DJV_UI_API void mouseReleaseEvent(ftk::MouseClickEvent&) override;

        private:
            FTK_PRIVATE();
        };
    }
}
