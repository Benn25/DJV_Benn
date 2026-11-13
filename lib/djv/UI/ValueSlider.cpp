// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the DJV project.

#include <djv/UI/ValueSlider.h>

#include <ftk/UI/ClipboardSystem.h>
#include <ftk/UI/DrawUtil.h>
#include <ftk/UI/LineEdit.h>
#include <ftk/Core/Context.h>

#include <algorithm>
#include <iomanip>
#include <optional>
#include <sstream>

namespace djv
{
    namespace ui
    {
        namespace
        {
            //! How far the pointer goes before a press is a drag rather than
            //! a click, in pixels at a scale of one.
            const int dragThreshold = 3;

            std::optional<double> parse(const std::string& text)
            {
                std::optional<double> out;
                try
                {
                    size_t i = 0;
                    const double value = std::stod(text, &i);
                    if (std::isfinite(value))
                    {
                        out = value;
                    }
                }
                catch (const std::exception&)
                {}
                return out;
            }
        }

        struct IValueSlider::Private
        {
            int precision = 3;

            struct SizeData
            {
                int margin = 0;
                int border = 0;
                int keyFocus = 0;
                int radius = 0;
                int dragThreshold = 0;
                ftk::FontInfo fontInfo;
                ftk::FontMetrics fontMetrics;
                ftk::Size2I sizeHint;
            };
            SizeData size;

            enum class Mode
            {
                None,
                Pressed,
                Drag
            };
            Mode mode = Mode::None;
            ftk::V2I pressPos;
            int dragX = 0;
            double dragValue = 0.0;

            std::shared_ptr<ftk::LineEdit> lineEdit;
            bool editing = false;
            std::shared_ptr<ftk::IWidgetPopup> popup;
        };

        void IValueSlider::_init(
            const std::shared_ptr<ftk::Context>& context,
            const std::string& objectName,
            const std::shared_ptr<IWidget>& parent)
        {
            IMouseWidget::_init(context, objectName, parent);
            FTK_P();
            setHStretch(ftk::Stretch::Expanding);
            _setMouseHoverEnabled(true);
            _setMousePressEnabled(true);

            // Typed into where the value is drawn.
            p.lineEdit = ftk::LineEdit::create(context, shared_from_this());
            p.lineEdit->setVisible(false);
            p.lineEdit->setCallbackOnFocusLost(true);
            p.lineEdit->setCallback(
                [this](const std::string& text)
                {
                    FTK_P();
                    if (!p.editing)
                        return;
                    if (const auto value = parse(text))
                    {
                        _setValue(value.value(), true);
                    }
                    _editEnd(true);
                });
            p.lineEdit->setEscapeCallback(
                [this]
                {
                    _editEnd(false);
                });
        }

        IValueSlider::IValueSlider() :
            _p(new Private)
        {}

        IValueSlider::~IValueSlider()
        {}

        int IValueSlider::getPrecision() const
        {
            return _p->precision;
        }

        void IValueSlider::setPrecision(int value)
        {
            FTK_P();
            if (value == p.precision)
                return;
            p.precision = value;
            setSizeUpdate();
            setDrawUpdate();
        }

        ftk::Size2I IValueSlider::getSizeHint() const
        {
            return _p->size.sizeHint;
        }

        void IValueSlider::setGeometry(const ftk::Box2I& value)
        {
            IMouseWidget::setGeometry(value);
            FTK_P();
            const int h = std::min(p.size.sizeHint.h, value.h());
            p.lineEdit->setGeometry(ftk::Box2I(
                value.min.x,
                value.min.y + (value.h() - h) / 2,
                value.w(),
                h));
        }

        void IValueSlider::setEnabled(bool value)
        {
            IMouseWidget::setEnabled(value);
            if (!value)
            {
                _editEnd(false);
            }
        }

        void IValueSlider::sizeHintEvent(const ftk::SizeHintEvent& event)
        {
            IMouseWidget::sizeHintEvent(event);
            FTK_P();
            const float s = event.displayScale;
            p.size.margin = event.style->getSizeRole(ftk::SizeRole::MarginInside, s);
            p.size.border = event.style->getSizeRole(ftk::SizeRole::Border, s);
            p.size.keyFocus = event.style->getSizeRole(ftk::SizeRole::KeyFocus, s);
            p.size.dragThreshold = static_cast<int>(dragThreshold * s);
            p.size.fontInfo = event.style->getFont(ftk::FontType::Regular, s);
            p.size.fontMetrics = event.fontSystem->getMetrics(p.size.fontInfo);
            const int h = p.size.fontMetrics.lineHeight + (p.size.margin + p.size.keyFocus) * 2;
            p.size.radius = h / 4;
            const ftk::Size2I textSize = event.fontSystem->getSize(
                _format(-88888.0),
                p.size.fontInfo);
            p.size.sizeHint = ftk::Size2I(
                std::max(
                    event.style->getSizeRole(ftk::SizeRole::Slider, s),
                    textSize.w + p.size.margin * 4),
                h);
        }

        void IValueSlider::drawEvent(const ftk::Box2I& drawRect, const ftk::DrawEvent& event)
        {
            IMouseWidget::drawEvent(drawRect, event);
            FTK_P();
            const bool enabled = isEnabled();
            const ftk::Box2I& g = getGeometry();
            const int h = std::min(p.size.sizeHint.h, g.h());
            const ftk::Box2I box(g.min.x, g.min.y + (g.h() - h) / 2, g.w(), h);
            const int r = std::min(p.size.radius, box.h() / 2);

            // The bar, filled from the start of the range to the value.
            event.render->drawMesh(
                ftk::rect(box, r),
                event.style->getColorRole(ftk::ColorRole::Button, enabled));
            const double value = _getValue();
            const double min = _getMin();
            const double max = _getMax();
            if (max > min)
            {
                const double t = std::clamp((value - min) / (max - min), 0.0, 1.0);
                const int w = static_cast<int>(std::round(box.w() * t));
                if (w > 0)
                {
                    // Rounded where the bar is, square where it stops.
                    const int rf = std::min(r, w / 2);
                    const int rr = w >= box.w() - r ? rf : 0;
                    event.render->drawMesh(
                        ftk::rect(
                            ftk::Box2I(box.min.x, box.min.y, w, box.h()),
                            std::array<int, 4>{ rf, rr, rr, rf }),
                        event.style->getColorRole(ftk::ColorRole::Checked, enabled));
                }
            }
            if (Private::Mode::Drag == p.mode)
            {
                event.render->drawMesh(
                    ftk::rect(box, r),
                    event.style->getColorRole(ftk::ColorRole::Pressed));
            }
            else if (_isMouseInside())
            {
                event.render->drawMesh(
                    ftk::rect(box, r),
                    event.style->getColorRole(ftk::ColorRole::Hover));
            }
            event.render->drawMesh(
                ftk::border(box, p.size.border, r),
                event.style->getColorRole(ftk::ColorRole::Border, enabled));

            // The value, in the middle.
            if (!p.editing)
            {
                const std::string text = _format(value);
                const ftk::Size2I textSize = event.fontSystem->getSize(text, p.size.fontInfo);
                event.render->drawText(
                    event.fontSystem->getGlyphs(text, p.size.fontInfo),
                    p.size.fontMetrics,
                    ftk::V2I(
                        box.min.x + box.w() / 2 - textSize.w / 2,
                        box.min.y + box.h() / 2 - p.size.fontMetrics.lineHeight / 2),
                    event.style->getColorRole(ftk::ColorRole::Text, enabled));
            }
        }

        void IValueSlider::mouseLeaveEvent()
        {
            IMouseWidget::mouseLeaveEvent();
            setDrawUpdate();
        }

        void IValueSlider::mouseMoveEvent(ftk::MouseMoveEvent& event)
        {
            IMouseWidget::mouseMoveEvent(event);
            FTK_P();
            if (Private::Mode::Pressed == p.mode &&
                std::abs(event.pos.x - p.pressPos.x) > p.size.dragThreshold)
            {
                p.mode = Private::Mode::Drag;
                p.dragX = event.pos.x;
                setDrawUpdate();
            }
            if (Private::Mode::Drag == p.mode)
            {
                // Across the slider is across the range; Shift ten times
                // finer. Kept unrounded, so a slow drag still gets there.
                const double min = _getMin();
                const double max = _getMax();
                const int w = std::max(1, getGeometry().w());
                double scale = (max - min) / w;
                if (event.modifiers & static_cast<int>(ftk::KeyModifier::Shift))
                {
                    scale *= .1;
                }
                p.dragValue = std::clamp(
                    p.dragValue + (event.pos.x - p.dragX) * scale,
                    min,
                    max);
                p.dragX = event.pos.x;
                double value = p.dragValue;
                const double step = _getStep();
                if ((event.modifiers & static_cast<int>(ftk::KeyModifier::Control)) && step > 0.0)
                {
                    value = std::clamp(std::round(value / step) * step, min, max);
                }
                _setValue(value, false);
            }
        }

        void IValueSlider::mousePressEvent(ftk::MouseClickEvent& event)
        {
            FTK_P();
            if (ftk::MouseButton::Right == event.button)
            {
                event.accept = true;
                _showPopup();
                return;
            }
            IMouseWidget::mousePressEvent(event);
            if (ftk::MouseButton::Left == event.button)
            {
                p.mode = Private::Mode::Pressed;
                p.pressPos = event.pos;
                p.dragValue = std::clamp(_getValue(), _getMin(), _getMax());
                _pressed(true);
            }
        }

        void IValueSlider::mouseReleaseEvent(ftk::MouseClickEvent& event)
        {
            IMouseWidget::mouseReleaseEvent(event);
            FTK_P();
            const Private::Mode mode = p.mode;
            p.mode = Private::Mode::None;
            if (Private::Mode::None != mode)
            {
                _pressed(false);
                setDrawUpdate();
            }
            // A click that did not drag is for typing.
            if (Private::Mode::Pressed == mode)
            {
                _editBegin();
            }
        }

        void IValueSlider::scrollEvent(ftk::ScrollEvent& event)
        {
            // Control and the wheel; the wheel alone scrolls the panel.
            if ((event.modifiers & static_cast<int>(ftk::KeyModifier::Control)) &&
                event.value.y != 0.F && isEnabled())
            {
                event.accept = true;
                double step = _getStep();
                if (step <= 0.0)
                {
                    step = (_getMax() - _getMin()) / 100.0;
                }
                _setValue(
                    std::clamp(
                        _getValue() + (event.value.y > 0.F ? step : -step),
                        _getMin(),
                        _getMax()),
                    false);
            }
        }

        void IValueSlider::keyPressEvent(ftk::KeyEvent& event)
        {
            FTK_P();
            if (p.editing || !isEnabled())
                return;
            const bool control = event.modifiers & static_cast<int>(ftk::KeyModifier::Control);
            switch (event.key)
            {
            case ftk::Key::Backspace:
            case ftk::Key::Delete:
                if (_hasDefault() && !event.modifiers)
                {
                    event.accept = true;
                    _setValue(_getDefault(), true);
                }
                break;
            case ftk::Key::Minus:
            case ftk::Key::KeypadMinus:
                if (!event.modifiers && _getMin() < 0.0)
                {
                    event.accept = true;
                    _setValue(-_getValue(), true);
                }
                break;
            case ftk::Key::Return:
                if (!event.modifiers)
                {
                    event.accept = true;
                    _editBegin();
                }
                break;
            case ftk::Key::C:
                if (control)
                {
                    if (auto context = getContext())
                    {
                        event.accept = true;
                        context->getSystem<ftk::ClipboardSystem>()->setText(_format(_getValue()));
                    }
                }
                break;
            case ftk::Key::V:
                if (control)
                {
                    if (auto context = getContext())
                    {
                        if (const auto value = parse(
                            context->getSystem<ftk::ClipboardSystem>()->getText()))
                        {
                            event.accept = true;
                            _setValue(value.value(), true);
                        }
                    }
                }
                break;
            default: break;
            }
        }

        void IValueSlider::keyReleaseEvent(ftk::KeyEvent& event)
        {
            event.accept = false;
        }

        bool IValueSlider::_isDragging() const
        {
            return Private::Mode::Drag == _p->mode;
        }

        std::string IValueSlider::_format(double value) const
        {
            std::stringstream ss;
            if (_isInteger())
            {
                ss << std::llround(value);
            }
            else
            {
                ss << std::fixed << std::setprecision(_p->precision) << value;
            }
            return ss.str();
        }

        void IValueSlider::_editBegin()
        {
            FTK_P();
            if (p.editing || !isEnabled())
                return;
            p.editing = true;
            p.lineEdit->setText(_format(_getValue()));
            p.lineEdit->setVisible(true);
            p.lineEdit->takeKeyFocus();
            p.lineEdit->selectAll();
            setDrawUpdate();
        }

        void IValueSlider::_editEnd(bool)
        {
            FTK_P();
            if (!p.editing)
                return;
            p.editing = false;
            p.lineEdit->setVisible(false);
            setDrawUpdate();
        }

        void IValueSlider::_showPopup()
        {
            FTK_P();
            if (p.popup)
            {
                p.popup->close();
                p.popup.reset();
                return;
            }
            p.popup = _createPopup();
            if (p.popup)
            {
                std::weak_ptr<IValueSlider> weak(
                    std::static_pointer_cast<IValueSlider>(shared_from_this()));
                p.popup->setCloseCallback(
                    [weak]
                    {
                        if (auto widget = weak.lock())
                        {
                            widget->_p->popup.reset();
                        }
                    });
                p.popup->open(getWindow(), getGeometry());
            }
        }
    }
}
