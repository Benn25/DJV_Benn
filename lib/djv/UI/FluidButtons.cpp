// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the DJV project.

#include <djv/UI/FluidButtons.h>

#include <ftk/UI/DrawUtil.h>
#include <ftk/Core/Context.h>
#include <ftk/Core/FontSystem.h>
#include <ftk/Core/String.h>

#include <algorithm>

namespace djv
{
    namespace ui
    {
        namespace
        {
            //! The most of a text that fits a width, cut short with an
            //! ellipsis: the whole of it when it fits, nothing when not even
            //! one character does.
            std::string elideToWidth(
                const std::string& text,
                int width,
                const std::shared_ptr<ftk::FontSystem>& fontSystem,
                const ftk::FontInfo& fontInfo)
            {
                if (fontSystem->getSize(text, fontInfo).w <= width)
                {
                    return text;
                }
                // The most characters that fit, by halves: it is measured as
                // a panel is dragged narrower.
                size_t lo = 0;
                size_t hi = text.size();
                while (lo < hi)
                {
                    const size_t n = (lo + hi + 1) / 2;
                    if (fontSystem->getSize(ftk::elide(text, n, ftk::ElideMode::Right), fontInfo).w <= width)
                    {
                        lo = n;
                    }
                    else
                    {
                        hi = n - 1;
                    }
                }
                return lo > 0 ? ftk::elide(text, lo, ftk::ElideMode::Right) : std::string();
            }
        }

        void PushButton::_init(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            ftk::PushButton::_init(context, parent);
            setHStretch(ftk::Stretch::Expanding);
        }

        PushButton::PushButton()
        {}

        PushButton::~PushButton()
        {}

        std::shared_ptr<PushButton> PushButton::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<PushButton>(new PushButton);
            out->_init(context, parent);
            return out;
        }

        std::shared_ptr<PushButton> PushButton::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::string& text,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = create(context, parent);
            out->setText(text);
            return out;
        }

        ftk::Size2I PushButton::getSizeHint() const
        {
            // As narrow as its icon, or a little text; the expanding stretch
            // gives it the rest of the row.
            const ftk::Size2I hint = ftk::PushButton::getSizeHint();
            return ftk::Size2I(std::min(hint.w, _minWidth), hint.h);
        }

        void PushButton::sizeHintEvent(const ftk::SizeHintEvent& event)
        {
            ftk::PushButton::sizeHintEvent(event);
            const float s = event.displayScale;
            _margin = event.style->getSizeRole(ftk::SizeRole::MarginInside, s);
            _pad = event.style->getSizeRole(ftk::SizeRole::LabelPad, s);
            _keyFocus = event.style->getSizeRole(ftk::SizeRole::KeyFocus, s);
            _cornerRadius = event.style->getSizeRole(ftk::SizeRole::CornerRadius, s);
            _fontInfo = event.style->getFont(_font, s);
            _fontMetrics = event.fontSystem->getMetrics(_fontInfo);
            const int chrome = (_margin + _pad + _keyFocus) * 2;
            _minWidth = chrome + (_iconImage ?
                _iconImage->getWidth() :
                event.fontSystem->getSize("W...", _fontInfo).w);
        }

        void PushButton::drawEvent(const ftk::Box2I& drawRect, const ftk::DrawEvent& event)
        {
            const ftk::Box2I& g = getGeometry();
            if (g.w() >= ftk::PushButton::getSizeHint().w)
            {
                ftk::PushButton::drawEvent(drawRect, event);
                return;
            }

            // Narrower than the text: drawn as ftk does, the text cut short.
            ftk::IButton::drawEvent(drawRect, event);
            const ftk::TriMesh2F button = ftk::rect(g, _cornerRadius);
            const ftk::ColorRole colorRole = _checked ? _checkedRole : _buttonRole;
            if (colorRole != ftk::ColorRole::None)
            {
                event.render->drawMesh(button, event.style->getColorRole(colorRole));
            }
            if (_isMousePressed())
            {
                event.render->drawMesh(button, event.style->getColorRole(ftk::ColorRole::Pressed));
            }
            else if (_isMouseInside())
            {
                event.render->drawMesh(button, event.style->getColorRole(ftk::ColorRole::Hover));
            }
            if (showKeyFocus())
            {
                event.render->drawMesh(
                    ftk::border(g, _keyFocus, _cornerRadius),
                    event.style->getColorRole(ftk::ColorRole::KeyFocus));
            }
            const ftk::Box2I inside = ftk::margin(
                g,
                -(_margin + _pad + _keyFocus),
                -(_margin + _keyFocus),
                -(_margin + _pad + _keyFocus),
                -(_margin + _keyFocus));
            int x = inside.x();
            int textWidth = inside.w();
            if (_iconImage)
            {
                const ftk::Size2I& iconSize = _iconImage->getSize();
                event.render->drawImage(
                    _iconImage,
                    ftk::Box2I(
                        x,
                        inside.y() + inside.h() / 2 - iconSize.h / 2,
                        iconSize.w,
                        iconSize.h),
                    _getTextColor(event));
                x += iconSize.w;
                textWidth -= iconSize.w;
            }
            const std::string text = elideToWidth(
                _text,
                textWidth - _pad * 2,
                event.fontSystem,
                _fontInfo);
            if (!text.empty())
            {
                event.render->drawText(
                    event.fontSystem->getGlyphs(text, _fontInfo),
                    _fontMetrics,
                    ftk::V2I(
                        x + _pad,
                        inside.y() + inside.h() / 2 - _fontMetrics.lineHeight / 2),
                    _getTextColor(event));
            }
        }

        void ComboBox::_init(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            ftk::ComboBox::_init(context, parent);
            setHStretch(ftk::Stretch::Expanding);
        }

        ComboBox::ComboBox()
        {}

        ComboBox::~ComboBox()
        {}

        std::shared_ptr<ComboBox> ComboBox::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ComboBox>(new ComboBox);
            out->_init(context, parent);
            return out;
        }

        std::shared_ptr<ComboBox> ComboBox::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::vector<ftk::ComboBoxItem>& items,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = create(context, parent);
            out->setItems(items);
            return out;
        }

        std::shared_ptr<ComboBox> ComboBox::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::vector<std::string>& items,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = create(context, parent);
            out->setItems(items);
            return out;
        }

        ftk::Size2I ComboBox::getSizeHint() const
        {
            const ftk::Size2I hint = ftk::ComboBox::getSizeHint();
            return ftk::Size2I(std::min(hint.w, _minWidth), hint.h);
        }

        void ComboBox::setGeometry(const ftk::Box2I& value)
        {
            ftk::ComboBox::setGeometry(value);
            if (value.w() != _width)
            {
                _width = value.w();
                _elideUpdate();
            }
        }

        void ComboBox::sizeHintEvent(const ftk::SizeHintEvent& event)
        {
            ftk::ComboBox::sizeHintEvent(event);
            _fontInfo = event.style->getFont(getFont(), event.displayScale);
            // What is not the text: the margins and the arrow. The ftk hint
            // is the chrome and the widest item as elided now.
            int textWidth = 0;
            for (const auto& item : getItems())
            {
                const std::string text = getElide() > 0 ?
                    ftk::elide(item.text, getElide(), getElideMode()) :
                    item.text;
                textWidth = std::max(textWidth, event.fontSystem->getSize(text, _fontInfo).w);
            }
            _chrome = std::max(0, ftk::ComboBox::getSizeHint().w - textWidth);
            _minWidth = _chrome + event.fontSystem->getSize("W...", _fontInfo).w;
            // The items may have changed.
            _elideUpdate();
        }

        void ComboBox::_elideUpdate()
        {
            auto context = getContext();
            if (!context || _width <= 0)
                return;
            auto fontSystem = context->getSystem<ftk::FontSystem>();
            const int available = _width - _chrome;
            // The longest elide that lets every item fit, the whole of them
            // when they do.
            size_t longest = 0;
            int widest = 0;
            for (const auto& item : getItems())
            {
                longest = std::max(longest, item.text.size());
                widest = std::max(widest, fontSystem->getSize(item.text, _fontInfo).w);
            }
            size_t elide = 0;
            if (widest > available)
            {
                // By halves: it is measured as a panel is dragged narrower.
                const auto fits = [this, &fontSystem, available](size_t n)
                {
                    for (const auto& item : getItems())
                    {
                        if (fontSystem->getSize(
                            ftk::elide(item.text, n, ftk::ElideMode::Right), _fontInfo).w > available)
                        {
                            return false;
                        }
                    }
                    return true;
                };
                size_t lo = 1;
                size_t hi = longest;
                while (lo < hi)
                {
                    const size_t n = (lo + hi + 1) / 2;
                    if (fits(n))
                    {
                        lo = n;
                    }
                    else
                    {
                        hi = n - 1;
                    }
                }
                elide = lo;
            }
            if (elide != getElide())
            {
                setElide(elide, ftk::ElideMode::Right);
            }
        }
    }
}
