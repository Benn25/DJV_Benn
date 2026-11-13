// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the DJV project.

#include <djv/UI/CheckBox.h>

#include <ftk/UI/IWindow.h>

namespace djv
{
    namespace ui
    {
        namespace
        {
            //! The check boxes under a point, shown and not clipped away.
            void findCheckBoxes(
                const std::shared_ptr<ftk::IWidget>& widget,
                const ftk::V2I& pos,
                std::vector<std::shared_ptr<CheckBox> >& out)
            {
                for (const auto& child : widget->getChildren())
                {
                    if (!child->isVisible(false) || child->isClipped())
                        continue;
                    if (auto checkBox = std::dynamic_pointer_cast<CheckBox>(child))
                    {
                        if (ftk::contains(checkBox->getGeometry(), pos))
                        {
                            out.push_back(checkBox);
                        }
                        continue;
                    }
                    findCheckBoxes(child, pos, out);
                }
            }
        }

        void CheckBox::_init(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            ftk::CheckBox::_init(context, parent);
        }

        CheckBox::CheckBox()
        {}

        CheckBox::~CheckBox()
        {}

        std::shared_ptr<CheckBox> CheckBox::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<CheckBox>(new CheckBox);
            out->_init(context, parent);
            return out;
        }

        std::shared_ptr<CheckBox> CheckBox::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::string& text,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = create(context, parent);
            out->setText(text);
            return out;
        }

        void CheckBox::mouseMoveEvent(ftk::MouseMoveEvent& event)
        {
            ftk::CheckBox::mouseMoveEvent(event);
            if (!_sweep)
                return;
            event.accept = true;
            if (auto window = getWindow())
            {
                std::vector<std::shared_ptr<CheckBox> > checkBoxes;
                findCheckBoxes(window, event.pos, checkBoxes);
                for (const auto& checkBox : checkBoxes)
                {
                    if (checkBox.get() != this &&
                        checkBox->isEnabled() &&
                        checkBox->isChecked() != _sweepChecked)
                    {
                        checkBox->click();
                    }
                }
            }
        }

        void CheckBox::mousePressEvent(ftk::MouseClickEvent& event)
        {
            if (ftk::MouseButton::Left == event.button && 0 == event.modifiers && isEnabled())
            {
                // Toggled on the press, as Blender's are, rather than the
                // release: the state to sweep the others to is known as the
                // drag starts.
                ftk::IMouseWidget::mousePressEvent(event);
                if (acceptsKeyFocus())
                {
                    takeKeyFocus();
                }
                auto self = shared_from_this();
                click();
                _sweep = true;
                _sweepChecked = isChecked();
                setDrawUpdate();
                return;
            }
            ftk::CheckBox::mousePressEvent(event);
        }

        void CheckBox::mouseReleaseEvent(ftk::MouseClickEvent& event)
        {
            if (_sweep)
            {
                // Toggled already: not again as a click.
                _sweep = false;
                ftk::IMouseWidget::mouseReleaseEvent(event);
                setDrawUpdate();
                return;
            }
            ftk::CheckBox::mouseReleaseEvent(event);
        }
    }
}
