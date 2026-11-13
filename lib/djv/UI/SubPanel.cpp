// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the DJV project.

#include <djv/UI/SubPanel.h>

#include <ftk/UI/DrawUtil.h>
#include <ftk/UI/IButton.h>
#include <ftk/UI/RowLayout.h>

#include <optional>

namespace djv
{
    namespace ui
    {
        namespace
        {
            ftk::Color4F mix(const ftk::Color4F& a, const ftk::Color4F& b, float t)
            {
                return ftk::Color4F(
                    a.r + (b.r - a.r) * t,
                    a.g + (b.g - a.g) * t,
                    a.b + (b.b - a.b) * t,
                    a.a + (b.a - a.a) * t);
            }

            //! The header: the arrow, smaller than a panel's, and the title,
            //! a little dimmer, on a bar lower than a panel's. It only
            //! draws: the subpanel takes the press, to sweep its siblings.
            class Header : public ftk::IButton
            {
            protected:
                void _init(const std::shared_ptr<ftk::Context>& context)
                {
                    IButton::_init(context, "djv::ui::SubPanel::Header", nullptr);
                    _setMousePressEnabled(false);
                    setCheckable(true);
                    setIcon("BellowsClosed");
                    setCheckedIcon("BellowsOpen");
                    setHStretch(ftk::Stretch::Expanding);
                }

            public:
                static std::shared_ptr<Header> create(const std::shared_ptr<ftk::Context>& context)
                {
                    auto out = std::shared_ptr<Header>(new Header);
                    out->_init(context);
                    return out;
                }

                ftk::Size2I getSizeHint() const override
                {
                    return _sizeHint;
                }

                void sizeHintEvent(const ftk::SizeHintEvent& event) override
                {
                    IButton::sizeHintEvent(event);
                    const float s = event.displayScale;
                    _margin = event.style->getSizeRole(ftk::SizeRole::MarginInside, s);
                    _pad = event.style->getSizeRole(ftk::SizeRole::LabelPad, s);
                    _fontInfo = event.style->getFont(_font, s);
                    _fontMetrics = event.fontSystem->getMetrics(_fontInfo);
                    const ftk::Size2I textSize = event.fontSystem->getSize(_text, _fontInfo);
                    _iconSize = _fontMetrics.lineHeight * 3 / 4;
                    _sizeHint = ftk::Size2I(
                        _margin + _iconSize + _pad + textSize.w + _pad + _margin,
                        _fontMetrics.lineHeight + _margin * 2);
                }

                void drawEvent(const ftk::Box2I& drawRect, const ftk::DrawEvent& event) override
                {
                    IButton::drawEvent(drawRect, event);
                    const ftk::Box2I& g = getGeometry();
                    if (_isMousePressed())
                    {
                        event.render->drawRect(g, event.style->getColorRole(ftk::ColorRole::Pressed));
                    }
                    else if (_isMouseInside())
                    {
                        event.render->drawRect(g, event.style->getColorRole(ftk::ColorRole::Hover));
                    }
                    const ftk::Color4F textColor = mix(
                        event.style->getColorRole(ftk::ColorRole::Text, isEnabled()),
                        event.style->getColorRole(ftk::ColorRole::TextDisabled, isEnabled()),
                        .25F);
                    int x = g.min.x + _margin;
                    if (auto icon = _checked ? _checkedIconImage : _iconImage)
                    {
                        event.render->drawImage(
                            icon,
                            ftk::Box2I(
                                x,
                                g.min.y + g.h() / 2 - _iconSize / 2,
                                _iconSize,
                                _iconSize),
                            textColor);
                    }
                    x += _iconSize + _pad;
                    event.render->drawText(
                        event.fontSystem->getGlyphs(_text, _fontInfo),
                        _fontMetrics,
                        ftk::V2I(x, g.min.y + g.h() / 2 - _fontMetrics.lineHeight / 2),
                        textColor);
                }

            private:
                ftk::Size2I _sizeHint;
                int _margin = 0;
                int _pad = 0;
                int _iconSize = 0;
                ftk::FontInfo _fontInfo;
                ftk::FontMetrics _fontMetrics;
            };
        }

        struct SubPanel::Private
        {
            std::shared_ptr<Header> header;
            std::shared_ptr<IWidget> widget;
            std::shared_ptr<IWidget> toolWidget;
            std::shared_ptr<ftk::HorizontalLayout> headerLayout;
            std::shared_ptr<ftk::VerticalLayout> layout;
            std::function<void(bool)> openCallback;

            int inset = 0;
            int radius = 0;

            bool sweep = false;
            bool sweepOpen = false;
        };

        void SubPanel::_init(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "djv::ui::SubPanel", parent);
            FTK_P();

            p.header = Header::create(context);

            p.headerLayout = ftk::HorizontalLayout::create(context);
            p.headerLayout->setSpacingRole(ftk::SizeRole::None);
            p.header->setParent(p.headerLayout);

            p.layout = ftk::VerticalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(ftk::SizeRole::None);
            p.headerLayout->setParent(p.layout);
        }

        SubPanel::SubPanel() :
            _p(new Private)
        {}

        SubPanel::~SubPanel()
        {}

        std::shared_ptr<SubPanel> SubPanel::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<SubPanel>(new SubPanel);
            out->_init(context, parent);
            return out;
        }

        std::shared_ptr<SubPanel> SubPanel::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::string& text,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = create(context, parent);
            out->setText(text);
            return out;
        }

        const std::string& SubPanel::getText() const
        {
            return _p->header->getText();
        }

        void SubPanel::setText(const std::string& value)
        {
            _p->header->setText(value);
        }

        const std::shared_ptr<ftk::IWidget>& SubPanel::getWidget() const
        {
            return _p->widget;
        }

        void SubPanel::setWidget(const std::shared_ptr<IWidget>& value)
        {
            FTK_P();
            if (value == p.widget)
                return;
            if (p.widget)
            {
                p.widget->setParent(nullptr);
            }
            p.widget = value;
            if (p.widget)
            {
                p.widget->setParent(p.layout);
                p.widget->setVisible(p.header->isChecked());
            }
            setSizeUpdate();
            setDrawUpdate();
        }

        bool SubPanel::isOpen() const
        {
            return _p->header->isChecked();
        }

        void SubPanel::setOpen(bool value)
        {
            FTK_P();
            p.header->setChecked(value);
            if (p.widget)
            {
                p.widget->setVisible(value);
                if (p.openCallback)
                {
                    p.openCallback(value);
                }
            }
            setDrawUpdate();
        }

        void SubPanel::setOpenCallback(const std::function<void(bool)>& value)
        {
            _p->openCallback = value;
        }

        const std::shared_ptr<ftk::IWidget>& SubPanel::getToolWidget() const
        {
            return _p->toolWidget;
        }

        void SubPanel::setToolWidget(const std::shared_ptr<IWidget>& value)
        {
            FTK_P();
            if (p.toolWidget)
            {
                p.toolWidget->setParent(nullptr);
            }
            p.toolWidget = value;
            if (p.toolWidget)
            {
                p.toolWidget->setParent(p.headerLayout);
            }
        }

        ftk::Size2I SubPanel::getSizeHint() const
        {
            FTK_P();
            const ftk::Size2I hint = p.layout->getSizeHint();
            return ftk::Size2I(hint.w + p.inset * 2, hint.h + p.inset);
        }

        void SubPanel::setGeometry(const ftk::Box2I& value)
        {
            IWidget::setGeometry(value);
            FTK_P();
            // Inset at the sides, as Blender's are, and apart from the next
            // one below.
            p.layout->setGeometry(ftk::Box2I(
                value.min.x + p.inset,
                value.min.y,
                value.w() - p.inset * 2,
                value.h() - p.inset));
        }

        void SubPanel::sizeHintEvent(const ftk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            FTK_P();
            p.inset = event.style->getSizeRole(ftk::SizeRole::MarginSmall, event.displayScale);
            p.radius = event.style->getSizeRole(ftk::SizeRole::MarginInside, event.displayScale) * 2;
        }

        void SubPanel::mouseMoveEvent(ftk::MouseMoveEvent& event)
        {
            IWidget::mouseMoveEvent(event);
            FTK_P();
            if (!p.sweep)
                return;
            event.accept = true;
            // The others of the same panel only, not those of a panel inside
            // one of them or of another panel.
            if (auto parent = getParent())
            {
                for (const auto& child : parent->getChildren())
                {
                    auto other = std::dynamic_pointer_cast<SubPanel>(child);
                    if (other &&
                        other.get() != this &&
                        other->isVisible(false) &&
                        other->isOpen() != p.sweepOpen &&
                        ftk::contains(other->_p->header->getGeometry(), event.pos))
                    {
                        other->setOpen(p.sweepOpen);
                    }
                }
            }
        }

        void SubPanel::mousePressEvent(ftk::MouseClickEvent& event)
        {
            IWidget::mousePressEvent(event);
            FTK_P();
            if (ftk::MouseButton::Left == event.button &&
                !event.accept &&
                ftk::contains(p.header->getGeometry(), event.pos))
            {
                event.accept = true;
                setOpen(!isOpen());
                p.sweep = true;
                p.sweepOpen = isOpen();
            }
        }

        void SubPanel::mouseReleaseEvent(ftk::MouseClickEvent& event)
        {
            IWidget::mouseReleaseEvent(event);
            FTK_P();
            if (p.sweep)
            {
                event.accept = true;
                p.sweep = false;
            }
        }

        void SubPanel::drawEvent(const ftk::Box2I& drawRect, const ftk::DrawEvent& event)
        {
            IWidget::drawEvent(drawRect, event);
            FTK_P();
            // A box of its own, a shade off the panel, the header a shade
            // off that: the panel's header color brought most of the way to
            // the panel's.
            const ftk::Box2I box = p.layout->getGeometry();
            const ftk::Color4F window = event.style->getColorRole(ftk::ColorRole::Window);
            const ftk::Color4F base = event.style->getColorRole(ftk::ColorRole::Base);
            const ftk::Color4F header = event.style->getColorRole(ftk::ColorRole::Header);
            event.render->drawMesh(ftk::rect(box, p.radius), mix(window, base, .35F));
            const ftk::Box2I& h = p.headerLayout->getGeometry();
            const bool open = p.header->isChecked() && p.widget;
            event.render->drawMesh(
                ftk::rect(h, open ?
                    std::array<int, 4>{ p.radius, p.radius, 0, 0 } :
                    std::array<int, 4>{ p.radius, p.radius, p.radius, p.radius }),
                mix(window, header, .55F));
        }
    }
}
